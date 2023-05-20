## 一、Lab6 前言
操作系统实验的最后一篇笔记，不说什么了。本文主要讲了 Shell 的实现机制，管道通信略有说明。
<!-- more -->

## 二、Shell 程序的启动
这次我们还要回到 Init/init.c 文件。我们的 MOS 的所有实验都结束之后，`mips_init` 函数应该是这样的
```c
void mips_init() {
	printk("init.c:\tmips_init() is called\n");

	// lab2:
	mips_detect_memory();
	mips_vm_init();
	page_init();

	// lab3:
	env_init();

	// lab6:
	ENV_CREATE(user_icode);  // This must be the first env!

	// lab5:
	ENV_CREATE(fs_serv);  // This must be the second env!

	// lab3:
	kclock_init();
	enable_irq();
	while (1) {
	}
}
```

其中我们使用 `ENV_CREATE` 创建了两个用户进程。这两个进程的代码在编译时便写入了内核 ELF 文件中。其中第二个进程 `fs_serv` 就是 Lab5 中用到的文件系统服务进程；而第一个进程 `user_icode` 则是整个操作系统中除文件系统服务进程外所有进程的共同祖先进程，该进程便用于启动 Shell 进程。`user_icode` 或为 “user init code” 之意。

对应的文件位于 user/icode.c 中。首先读取 motd 文件并输出其内容。motd 即 “message of today” 的缩写。其实这一步只是为了打印欢迎信息罢了。
```c
int main() {
	int fd, n, r;
	char buf[512 + 1];

	debugf("icode: open /motd\n");
	if ((fd = open("/motd", O_RDONLY)) < 0) {
		user_panic("icode: open /motd: %d", fd);
	}

	debugf("icode: read /motd\n");
	while ((n = read(fd, buf, sizeof buf - 1)) > 0) {
		buf[n] = 0;
		debugf("%s\n", buf);
	}

	debugf("icode: close /motd\n");
	close(fd);
```

之后调用 `spawnl` 函数执行文件 `init.b`。`spawnl` 实际调用了 `spawn` 函数，该函数用于将磁盘中的文件加载到内存，并以此创建一个新进程。`spawn` 函数的内容让我们搁到后面，现在先来继续看 Shell 启动的过程。

执行的 `init.b` 文件的源代码在 user/init.c 中。其中前面一大部分都是测试 ELF 文件加载正确性的代码，就略过不讲了。重要的部分从这里开始。
```c
int main(int argc, char **argv) {
	int i, r, x, want;

	// omit...

	// stdin should be 0, because no file descriptors are open yet
	if ((r = opencons()) != 0) {
		user_panic("opencons: %d", r);
	}
```

首先调用了 `opencons` 打开一个终端文件。该函数位于 user/lib/console.c 中。内容很简单，只是申请了一个文件描述符，并设置 `fd->fd_dev_id = devcons.dev_id` 表示该文件属于终端文件，执行读写等等操作时使用终端提供的函数。

```c
int opencons(void) {
	int r;
	struct Fd *fd;

	if ((r = fd_alloc(&fd)) < 0) {
		return r;
	}
	if ((r = syscall_mem_alloc(0, fd, PTE_D | PTE_LIBRARY)) < 0) {
		return r;
	}
	fd->fd_dev_id = devcons.dev_id;
	fd->fd_omode = O_RDWR;
	return fd2num(fd);
}
```

对于终端的读写操作，我们也可以看一下。以写为例，对应的函数为 `cons_write`，实际可以看出只是通过系统调用实现读写而已。
```c
struct Dev devcons = {
    .dev_id = 'c',
    .dev_name = "cons",
    .dev_read = cons_read,
    .dev_write = cons_write,
    .dev_close = cons_close,
    .dev_stat = cons_stat,
};

int cons_write(struct Fd *fd, const void *buf, u_int n, u_int offset) {
	int r = syscall_print_cons(buf, n);
	if (r < 0) {
		return r;
	}
	return n;
}
```

让我们回到 user/init.c。因为调用 `opencons` 时系统内必定没有其他文件被打卡，所以该函数申请得到的文件描述符必定为 0。之后代码中又调用 `dup` 函数申请了一个新的文件描述符 1。该文件描述符是 0 的复制。也就是说，对这两个文件描述符进行读写等文件操作，都是对同一个文件（这里是终端文件）的操作。
```c
	// stdout
	if ((r = dup(0, 1)) < 0) {
		user_panic("dup: %d", r);
	}
```

`dup` 函数位于 user/lib/fd.c 中，本属于文件系统的内容，但 Lab5 中碍于篇幅没有讲解，这里我们讲解一下。`dup` 函数在最开始做了一系列准备工作，首先是根据已有的文件描述符 `oldfdnum` 找到对应的文件描述结构体。
```c
int dup(int oldfdnum, int newfdnum) {
	int i, r;
	void *ova, *nva;
	u_int pte;
	struct Fd *oldfd, *newfd;

	if ((r = fd_lookup(oldfdnum, &oldfd)) < 0) {
		return r;
	}
```

之后需要注意我们调用了 `close` 函数关闭了要复制的文件描述符 `newfdnum` 原本对应的文件（如果有的话）。只有这样我们才能将 `oldfdnum` 对应的文件复制给 `newfdnum`。最后我们通过 `fd2data` 函数根据描述符得到文件内容所在的地址位置。
```c
	close(newfdnum);
	newfd = (struct Fd *)INDEX2FD(newfdnum);
	ova = fd2data(oldfd);
	nva = fd2data(newfd);
```

之后我们要做的就是共享文件描述符和文件内容，这通过系统调用 `syscall_mem_map` 实现。

这一部分将 `oldfd` 所在的页映射到 `newfd` 所在的地址。需要注意这里的 `vpt[VPN(oldfd)] & (PTE_D | PTE_LIBRARY)` 表示为 `newfd` 设置与 `oldfd` 相同的可写可共享权限。
```c
	if ((r = syscall_mem_map(0, oldfd, 0, newfd, vpt[VPN(oldfd)] & (PTE_D | PTE_LIBRARY))) <
	    1) {
		goto err;
	}
```

对于文件内容也一样，对于有效的页（`pte & PTE_V`），我们同样需要进行映射。全部都映射完成后，返回新的文件描述符。

```c
	if (vpd[PDX(ova)]) {
		for (i = 0; i < PDMAP; i += BY2PG) {
			pte = vpt[VPN(ova + i)];

			if (pte & PTE_V) {
				// should be no error here -- pd is already allocated
				if ((r = syscall_mem_map(0, (void *)(ova + i), 0, (void *)(nva + i),
							 pte & (PTE_D | PTE_LIBRARY))) < 0) {
					goto err;
				}
			}
		}
	}

	return newfdnum;
```

很有趣的一点是这里 `dup` 函数中使用到了 `goto` 进行异常处理。当出错后，会跳转到 `err` 标签的位置，解除已经建立的映射关系。在之后我们还会多次遇到这种处理方法。这种方法的主要目的是在出错时释放已经分配的资源，优点是对多次申请不同资源的情况，可以很简洁地进行处理。这一点在 `dup` 函数中似乎不太明显，但等到了 `spawn` 函数中就可以看出来了。
```c
err:
	syscall_mem_unmap(0, newfd);

	for (i = 0; i < PDMAP; i += BY2PG) {
		syscall_mem_unmap(0, (void *)(nva + i));
	}

	return r;
}
```

让我们再回到 user/init.c。现在我们有了 0、1 两个文件描述符，分别表示了标准输入和标准输出。此后进程都会通过 `spawn` 或 `fork` 创建进程，这些新创建的进程也会继承两个标准输入输出，除非进程自己关闭。

最后是一个死循环，这个循环用于创建 Shell 进程。在调用 `spawnl` 创建 Shell 进程成功后，会调用 `wait` 等待 Shell 进程退出。因为处于循环中，所以当 Shell 进程退出后，又会创建一个新的 Shell 进程。

```c
	while (1) {
		debugf("init: starting sh\n");
		r = spawnl("sh.b", "sh", NULL);
		if (r < 0) {
			debugf("init: spawn sh: %d\n", r);
			return r;
		}
		wait(r);
	}
}
```

其中 `wait` 定义在 user/lib/wait.c 中。只是实现了一个简单的忙等，使得调用 `wait` 的函数被阻塞，直到 `envid` 对应的进程退出才继续执行。
```c
void wait(u_int envid) {
	volatile struct Env *e;

	e = &envs[ENVX(envid)];
	while (e->env_id == envid && e->env_status != ENV_FREE) {
		syscall_yield();
	}
}
```

这样 Shell 进程就启动了

## 三、Shell 原理
Shell 程序需要做的事就是这样：不断读取用户的命令输入，根据命令创建对应的进程，并实现进程间的通信。我们的 Shell 进程位于 user/sh.c。

首先打印了欢迎信息，这点就略过了。紧随其后是两个比较复杂的宏 `ARGBEGIN` 和 `ARGEND`。这两个宏和其中间的部分解析了命令中的选项。对于本函数来说，就是 `-i` 和 `-x`。
```c
int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
	int echocmds = 0;

	// omit...

	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND
```

> `ARGBEGIN` 和 `ARGEND` 的具体内容在 include/args.h 中，但是既然注释里已经说了 `you are not expected to understand this`，那我们还是跳过吧。但是我还想插一句嘴，注释里说道，这个参数解析宏是 `simple command line parser from Plan 9`。plan 9 是贝尔实验室 40 多年前开发的超前的实验性操作系统，没想到能看到这么老的代码……这可比 Lab2 中 30 年前的 [queue.h](https://github.com/torvalds/linux/blob/master/drivers/scsi/aic7xxx/queue.h) 还要离谱。

接下来的这一部分考虑了执行 Shell 脚本的情况。如果需要执行脚本，则关闭标准输入，改为文件作为输入。
```c
	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}
```

最后在循环中不断读入命令行并进行处理。对于交互界面，会首先输出提示符 `$`。随后读入一行命令。
```c
	for (;;) {
		if (interactive) {
			printf("\n$ ");
		}
		readline(buf, sizeof buf);
```

其中 `readline` 函数会逐字符读取。需要注意当标准输入未被重定向时（即从终端进行读取时），这一过程是和用户的输入同步的。当用户输入一个字符，`read` 才会读取一个字符，否则会被阻塞。
```c
void readline(char *buf, u_int n) {
	int r;
	for (int i = 0; i < n; i++) {
		if ((r = read(0, buf + i, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}
```

阻塞的实现也不过是一个忙等而已
```c
int cons_read(struct Fd *fd, void *vbuf, u_int n, u_int offset) {
	int c;

	if (n == 0) {
		return 0;
	}

	while ((c = syscall_cgetc()) == 0) {
		syscall_yield();
	}

	// omit...
}
```

之后，`readline` 中值得注意的是这里进行了退格的处理。所谓退格键就是 backspace 键，当按下该键后，就会在串口中产生一个退格符 `\b`。接下来我们的操作就很清楚了，只需要将之前读入的一个字符清除即可。另外对于终端显示，我们也要产生删除上一个字符的效果，这一效果可以通过向串口输出一个退格符实现 `printf("\b")`。
```c
		if (buf[i] == '\b' || buf[i] == 0x7f) {
			if (i > 0) {
				i -= 2;
			} else {
				i = -1;
			}
			if (buf[i] != '\b') {
				printf("\b");
			}
		}
```

> 哪里来的 `printf`？我们都知道之前用户进程只使用 `debugf` 进行输出，其实 `printf` 的实现也一样，该函数位于 user/lib/fprintf.c 中，是在 Lab5 时添加的，当时我怎么没注意到。

再之后，`readline` 判断是否读到换行符，是的话则退出，完成一行命令的读入。
```c
		if (buf[i] == '\r' || buf[i] == '\n') {
			buf[i] = 0;
			return;
		}
	}
```

在循环之外还对过长的命令进行了处理。在这一部分会不断读入剩下的命令，并返回空字符串。
```c
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}
```

回到 Shell 进程的 `main` 函数。接着我们会对读入的命令进行处理，首先忽略以 `#` 开头的注释，接着在 `echocmds` 模式下输出读入的命令。这些没有什么好说的，再之后的部分才是 Shell 的重点。
```c
		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
```

这里我们调用 `fork` 复制了一个 Shell 进程，执行 `runcmd` 函数来运行命令，而原本的 Shell 进程则等待该新复制的进程结束。
```c
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
		if (r == 0) {
			runcmd(buf);
			exit();
		} else {
			wait(r);
		}
	}
	return 0;
}
```

对于新复制的 Shell 进程，我们查看一下 `runcmd` 函数，这个函数以及 `parsecmd` 函数是 Shell 的核心。首先调用一次 `gettoken`，这将把 `s` 设定为要解析的字符串。`gettoken` 函数似乎较为复杂，涉及到字符串的解析，就不深入了。
```c
void runcmd(char *s) {
	gettoken(s, 0);
```

接着我们需要调用 `parsecmd` 将完整的字符串解析。解析的参数返回到 `argv`，参数的数量返回为 `argc`。
```c
	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
```

另外 `parsecmd` 还会返回 `rightpipe`。对于该值的理解需要从头到尾梳理一下 Shell 解析并执行命令的流程。首先为了方便讲述，我要定义三个名词。第一个是**主 Shell 进程**，即执行了 `main` 部分代码的 Shell 进程；第二个是**执行命令的 Shell 进程**，即调用 `spawn` 创建执行命令的进程；第三个是**命令进程**，即被 `spawn` 创建的进程。

那么我们就来分析一下解析和执行命令的过程。首先读到命令后，主 Shell 进程会调用 `fork` 创建一个执行命令的 Shell 进程。该进程会执行命令，创建命令进程。但问题是命令中可能出现管道 `|`，这就需要创建两个或多个命令进程。这时我们并不通过循环之类的方法多次调用 `spawn`，而是 `fork` 出一个新的执行命令的进程，由这个新的进程调用 `spawn` 根据管道右侧的命令创建命令进程；而原来的进程则根据管道左侧的命令创建。

`parsecmd` 函数完成了上述流程中 “`fork` 出新的执行命令的进程” 的任务。`rightpipe` 表示的就是 `fork` 创建的执行命令的 Shell 进程的进程 id。

当然除此之外该函数还完成了其他一些工作。比如将完整的命令字符串解析为参数数组；完成标准输入输出重定向到文件；为命令中管道符号 `|` 两侧的命令创建管道并将管道重定向到标准输入输出。

根据上面的内容，我们查看一下 `parsecmd` 函数的内容。该函数会不断解析命令，直到解析到命令字符串的末尾（`gettoken` 返回值为 0）则退出。
```c
int parsecmd(char **argv, int *rightpipe) {
	int argc = 0;
	while (1) {
		char *t;
		int fd, r;
		int c = gettoken(0, &t);
		switch (c) {
		case 0:
			return argc;
```

对于解析得到的是一般单词的情况，我们为参数数组 `argv` 添加一个参数。
```c
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
```

对于输入或输出重定向的情况，我们获取下一个 token，打开对应的文件，调用 `dup` 设定 0 或 1 为该文件的文件描述符（原本的标准输入输出被 “挤掉”），并关闭原来打开的文件描述符。
```c
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			/* Exercise 6.5: Your code here. (1/3) */
			fd = open(t, O_RDONLY);
			dup(fd, 0);
			close(fd);
			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, dup it onto fd 1, and then close the original fd.
			/* Exercise 6.5: Your code here. (2/3) */
			fd = open(t, O_WRONLY);
			dup(fd, 1);
			close(fd);
			break;
```

最后是重点部分，当读到的 token 为 `|` 时，我们就需要创建一个管道，并调用 `fork` 分出一个执行命令的 Shell 进程。并将 `fork` 的返回值传给 `*rightpipe`。此后位于管道左侧的进程完成命令的解析直接返回，这样位于管道左侧的进程获得的就一定是位于管道右侧的进程的进程 id。而位于管道右侧的进程则从继续解析剩下的命令，这里通过递归调用 `parsecmd` 实现。

对于创建的管道文件，我们同样要将其重定向到标准输入输出。对于管道左侧的进程，需要重定向其标准输出；而对于管道右侧的进程，则需要重定向其标准输入。
```c
		case '|':;
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
			pipe(p);
			*rightpipe = fork();
			if (*rightpipe == 0) {
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv, rightpipe);
			}  else if (*rightpipe > 0) {
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				return argc;
			}
			break;
		}
	}

	return argc;
}
```

现在回到 `runcmd` 函数。首先注意此时 `runcmd` 中可能会有多个 Shell 进程并发地执行，他们都具有不同的参数数组，同时各自的 `rightpipe` 变量中都保存了位于其右侧的进程的进程 id。

首先当参数数量为 0 时，说明根本没有命令，直接返回即可。否则需要调用 spawn 函数创建对应进程，并为其传入参数。注意这里我们没有向 `spawn` 中传入 `argc` 参数，而是通过设置 `argv[argc] = 0` 来简介表示 `argv` 的大小。

```c
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;

	int child = spawn(argv[0], argv);
```

之后，因为我们在执行命令的 Shell 进程中打开的文件都只是为了继承给 `spawn` 创建的进程，所以此时就要关闭当前进程的所有文件
```c
	close_all();
```

调用 `spawn` 后我们处理其返回值 `child`，对于父进程，需要等待子进程，即命令进程结束后继续执行
```c
	if (child >= 0) {
		wait(child);
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
```

最后对于存在管道通信的情况，还需要等待管道右侧的进程先结束。因为假设不等待，则在管道左侧的进程退出之后，管道右侧的进程就不能再通过管道读取信息了。
```c
	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}
```

这样 Shell 的机制就讲完了。关于进程运行的部分我们还需要考察一下 `spawn` 函数的实现。

## 四、spawn 函数的实现
`spawn` 函数位于 user/lib/spawn.c 中。还有一个可变参数的版本 `spawnl`，只是简单地调用了 `spawn` 函数。
```c
int spawnl(char *prog, char *args, ...) {
	// Thanks to MIPS calling convention, the layout of arguments on the stack
	// are straightforward.
	return spawn(prog, &args);
}
```

接下来我们考察一下 `spawn` 函数。首先要明确 `spawn` 函数的功能是根据磁盘文件创建一个进程。那么首先要做的就是将文件内容加载到内存中。

于是我们根据传入的文件路径参数 `prog` 打开文件，并首先通过 `readn` 函数读取其文件头的信息。
```c
int spawn(char *prog, char **argv) {
	// Step 1: Open the file 'prog' (the path of the program).
	// Return the error if 'open' fails.
	int fd;
	if ((fd = open(prog, O_RDONLY)) < 0) {
		return fd;
	}

	// Step 2: Read the ELF header (of type 'Elf32_Ehdr') from the file into 'elfbuf' using
	// 'readn()'.
	// If that fails (where 'readn' returns a different size than expected),
	// set 'r' and 'goto err' to close the file and return the error.
	int r;
	u_char elfbuf[512];
	
	/* Exercise 6.4: Your code here. (1/6) */
	if ((r = readn(fd, elfbuf, sizeof(Elf32_Ehdr))) < 0 || r != sizeof(Elf32_Ehdr)) {
		goto err;
	}
```

接着我们要将文件头转换为 `Elf32_Ehdr` 结构体的格式。我们通过 `elf_from` 完成这一步骤，该函数也在 Lab3 中使用过，在转换之前检查了文件头格式的有效性。（实际上 `spawn` 函数的实现也与 Lab3 中的 `load_icode` 类似，后者各位如果还记得的话，应该知道是用于加载编译器写进操作系统的程序的。）
```c
	const Elf32_Ehdr *ehdr = elf_from(elfbuf, sizeof(Elf32_Ehdr));
	if (!ehdr) {
		r = -E_NOT_EXEC;
		goto err;
	}
```

我们从 ELF 文件头中读取了程序入口信息。（虽然不知道有什么意义，似乎又是遗留没有修改的代码。）
```c
	u_long entrypoint = ehdr->e_entry;
```

接着使用系统调用 `syscall_exofork` 创建一个新的进程。注意这里不需要像 `fork` 一样创建后判断 `child` 的值来区分父子进程。因为在 `spawn` 之后的内容中我们会替换子进程的代码和数据，不会再从此处继续执行。
```c
	// Step 3: Create a child using 'syscall_exofork()' and store its envid in 'child'.
	// If the syscall fails, set 'r' and 'goto err'.
	u_int child;
	/* Exercise 6.4: Your code here. (2/6) */
	child = syscall_exofork();
	if (child < 0) {
		r = child;
		goto err;
	}
```

随后在父进程中，调用 `init_stack` 完成子进程栈的初始化。
```c
	// Step 4: Use 'init_stack(child, argv, &sp)' to initialize the stack of the child.
	// 'goto err1' if that fails.
	u_int sp;
	/* Exercise 6.4: Your code here. (3/6) */
	if ((r = init_stack(child, argv, &sp)) < 0) {
		goto err1;
	}
```

在 `init_stack` 函数中，我们要将 `argc`、`argv` 的数据写入要初始化的进程的栈中，完成进程栈的初始化。具体来说，我们首先要按一定格式将 `argc`、`argv` 写入 `UTEMP` 页（这个地址就在 `UCOW` 的下面，用途也和 `UCOW` 那一页类似，详见 Lab4）中，再将 `UTEMP` 映射到的物理页转而映射到要初始化进程的栈的位置。

首先统计一下所有参数字符串的长度。
```c
int init_stack(u_int child, char **argv, u_int *init_sp) {
	int argc, i, r, tot;
	char *strings;
	u_int *args;

	// Count the number of arguments (argc)
	// and the total amount of space needed for strings (tot)
	tot = 0;
	for (argc = 0; argv[argc]; argc++) {
		tot += strlen(argv[argc]) + 1;
	}
```

计算参数所需内存是否超过了一页的大小，如果超过直接返回异常。由这部分也可以大致看出栈的结构（`argc(4)+**argv(4)+*argv[i](4*(argc+1))+sum(len(argv[i])+1)`，其中argc+1 中的 1 为表示最后一个参数的 `argv[argc] = 0`，详见 `runcmd` 函数。）
```c
	// Make sure everything will fit in the initial stack page
	if (ROUND(tot, 4) + 4 * (argc + 3) > BY2PG) {
		return -E_NO_MEM;
	}
```

这一部分确定了字符串和指针数组的地址，并使用系统调用申请了 UTEMP 所在的页。
```c
	// Determine where to place the strings and the args array
	strings = (char *)(UTEMP + BY2PG) - tot;
	args = (u_int *)(UTEMP + BY2PG - ROUND(tot, 4) - 4 * (argc + 1));

	if ((r = syscall_mem_alloc(0, (void *)UTEMP, PTE_D)) < 0) {
		return r;
	}
```

这一部分复制了所有参数字符串
```c
	// Copy the argument strings into the stack page at 'strings'
	char *ctemp, *argv_temp;
	u_int j;
	ctemp = strings;
	for (i = 0; i < argc; i++) {
		argv_temp = argv[i];
		for (j = 0; j < strlen(argv[i]); j++) {
			*ctemp = *argv_temp;
			ctemp++;
			argv_temp++;
		}
		*ctemp = 0;
		ctemp++;
	}
```

这一部分设置了指针数组的内容
```c
	// Initialize args[0..argc-1] to be pointers to these strings
	// that will be valid addresses for the child environment
	// (for whom this page will be at USTACKTOP-BY2PG!).
	ctemp = (char *)(USTACKTOP - UTEMP - BY2PG + (u_int)strings);
	for (i = 0; i < argc; i++) {
		args[i] = (u_int)ctemp;
		ctemp += strlen(argv[i]) + 1;
	}
```

这里代码和注释的内容似乎不符，不知为何。
```c
	// Set args[argc] to 0 to null-terminate the args array.
	ctemp--;
	args[argc] = (u_int)ctemp;
```

最后设置 `argc` 的值和 `argv` 数组指针的内容
```c
	// Push two more words onto the child's stack below 'args',
	// containing the argc and argv parameters to be passed
	// to the child's main() function.
	u_int *pargv_ptr;
	pargv_ptr = args - 1;
	*pargv_ptr = USTACKTOP - UTEMP - BY2PG + (u_int)args;
	pargv_ptr--;
	*pargv_ptr = argc;
```

返回栈帧的初始地址
```c
	// Set *init_sp to the initial stack pointer for the child
	*init_sp = USTACKTOP - UTEMP - BY2PG + (u_int)pargv_ptr;
```

将 `UTEMP` 页映射到用户栈真正应该处于的地址，如果操作失败通过 `goto` 进行异常处理。
```c
	if ((r = syscall_mem_map(0, (void *)UTEMP, child, (void *)(USTACKTOP - BY2PG), PTE_D)) <
	    1) {
		goto error;
	}
	if ((r = syscall_mem_unmap(0, (void *)UTEMP)) < 0) {
		goto error;
	}

	return 0;

error:
	syscall_mem_unmap(0, (void *)UTEMP);
	return r;
}
```

这样就完成了程序栈的初始化。让我们回到 `spawn`。接下来我们遍历整个 ELF 头的程序段，将程序段的内容读到内存中。
```c
	// Step 5: Load the ELF segments in the file into the child's memory.
	// This is similar to 'load_icode()' in the kernel.
	size_t ph_off;
	ELF_FOREACH_PHDR_OFF (ph_off, ehdr) {
		// Read the program header in the file with offset 'ph_off' and length
		// 'ehdr->e_phentsize' into 'elfbuf'.
		// 'goto err1' on failure.
		// You may want to use 'seek' and 'readn'.
		/* Exercise 6.4: Your code here. (4/6) */
		if ((r = seek(fd, ph_off)) < 0 || (r = readn(fd, elfbuf, ehdr->e_phentsize)) < 0) {
			goto err1;
		}
```

如果是需要加载的程序段，则首先根据程序段相对于文件的偏移得到其在内存中映射到的地址，接着就和 Lab3 的 `load_icode` 函数一样，调用 `elf_load_seg` 将程序段加载到适当的位置。
```c
		Elf32_Phdr *ph = (Elf32_Phdr *)elfbuf;
		if (ph->p_type == PT_LOAD) {
			void *bin;
			// Read and map the ELF data in the file at 'ph->p_offset' into our memory
			// using 'read_map()'.
			// 'goto err1' if that fails.
			/* Exercise 6.4: Your code here. (5/6) */
			if ((r = read_map(fd, ph->p_offset, &bin)) < 0) {
				goto err1;
			}

			// Load the segment 'ph' into the child's memory using 'elf_load_seg()'.
			// Use 'spawn_mapper' as the callback, and '&child' as its data.
			// 'goto err1' if that fails.
			/* Exercise 6.4: Your code here. (6/6) */
			if ((r = elf_load_seg(ph, bin, spawn_mapper, &child)) < 0) {
				goto err1;
			}
		}
	}
	close(fd);
```

但还是要注意和 `load_icode` 的区别。此处我们设定的回调函数为 `spawn_mapper` 而非 `load_icode_mapper`。通过下面的对比可以看出，因为 `spawn_mapper` 处于用户态，所以使用了很多系统调用，同时我传入的是子进程的进程 id 而非进程控制块。
```c
// user/lib/spawn.c
static int spawn_mapper(void *data, u_long va, size_t offset, u_int perm, const void *src,
			size_t len) {
	u_int child_id = *(u_int *)data;
	try(syscall_mem_alloc(child_id, (void *)va, perm));
	if (src != NULL) {
		int r = syscall_mem_map(child_id, (void *)va, 0, (void *)UTEMP, perm | PTE_D);
		if (r) {
			syscall_mem_unmap(child_id, (void *)va);
			return r;
		}
		memcpy((void *)(UTEMP + offset), src, len);
		return syscall_mem_unmap(0, (void *)UTEMP);
	}
	return 0;
}


// kern/env.c
static int load_icode_mapper(void *data, u_long va, size_t offset, u_int perm, const void *src,
			     size_t len) {
	struct Env *env = (struct Env *)data;
	struct Page *p;
	int r;

	/* Step 1: Allocate a page with 'page_alloc'. */
	/* Exercise 3.5: Your code here. (1/2) */
	if ((r = page_alloc(&p)) != 0) {
		return r;
	}

	/* Step 2: If 'src' is not NULL, copy the 'len' bytes started at 'src' into 'offset' at this
	 * page. */
	// Hint: You may want to use 'memcpy'.
	if (src != NULL) {
		/* Exercise 3.5: Your code here. (2/2) */
		memcpy((void *)(page2kva(p) + offset), src, len);
	}

	/* Step 3: Insert 'p' into 'env->env_pgdir' at 'va' with 'perm'. */
	return page_insert(env->env_pgdir, env->env_asid, p, va, perm);
}
```

这样就将程序加载到了新创建的进程的适当位置了。之后设定栈帧，父子进程共享 `USTACKTOP` 地址之下的数据，这一部分和 `duppage` 的操作很相似，只不过这里不共享程序部分。
```c
	struct Trapframe tf = envs[ENVX(child)].env_tf;
	tf.cp0_epc = entrypoint;
	tf.regs[29] = sp;
	if ((r = syscall_set_trapframe(child, &tf)) != 0) {
		goto err2;
	}

	// Pages with 'PTE_LIBRARY' set are shared between the parent and the child.
	for (u_int pdeno = 0; pdeno <= PDX(USTACKTOP); pdeno++) {
		if (!(vpd[pdeno] & PTE_V)) {
			continue;
		}
		for (u_int pteno = 0; pteno <= PTX(~0); pteno++) {
			u_int pn = (pdeno << 10) + pteno;
			u_int perm = vpt[pn] & ((1 << PGSHIFT) - 1);
			if ((perm & PTE_V) && (perm & PTE_LIBRARY)) {
				void *va = (void *)(pn << PGSHIFT);

				if ((r = syscall_mem_map(0, va, child, va, perm)) < 0) {
					debugf("spawn: syscall_mem_map %x %x: %d\n", va, child, r);
					goto err2;
				}
			}
		}
	}
```

最后设定子进程为运行状态以将其加入进程调度队列，实现子进程的创建。
```c
	if ((r = syscall_set_env_status(child, ENV_RUNNABLE)) < 0) {
		debugf("spawn: syscall_set_env_status %x: %d\n", child, r);
		goto err2;
	}
	return child;
```

在最后则是异常处理部分，可以看出在 `spawn` 函数中因为分配的资源数量较多，异常处理部分也变复杂了。这里就可以看出使用 `goto` 进行异常处理的优点了。只需要按反向的顺序写出资源的释放函数，在异常时只需要跳转到对应位置就可以穿透标签，不断释放所有的资源。这和 Lab5 中我们解释过的 `switch` 的穿透类似。
```c
err2:
	syscall_env_destroy(child);
	return r;
err1:
	syscall_env_destroy(child);
err:
	close(fd);
	return r;
}
```

这样，`spawn` 函数也终于讲解完了。

## 五、管道通信的实现
最后还要讲一下管道。各位应该都清楚，管道是一种特殊的文件，它没有在磁盘中占用空间，所有的数据都存储在内存中。对于 MOS 来说，就是不需要与文件系统服务进程进行通讯，直接在内存中处理读写等等操作。

管道的相关操作位于 user/lib/pipe.c 中。创建管道的函数为 `pipe`。管道创建时总会返回两个文件描述符，用于对同一块内存的读写操作。

首先就是简单地申请文件描述符，同时申请用于表示文件内容的内存空间。另外对第二个文件描述符 `fd1` 还需要将该文件对应的内存空间映射到 `fd0` 对应的空间。这样两个文件描述符才能共享同一块物理内存。
```c
int pipe(int pfd[2]) {
	int r;
	void *va;
	struct Fd *fd0, *fd1;

	/* Step 1: Allocate the file descriptors. */
	if ((r = fd_alloc(&fd0)) < 0 || (r = syscall_mem_alloc(0, fd0, PTE_D | PTE_LIBRARY)) < 0) {
		goto err;
	}

	if ((r = fd_alloc(&fd1)) < 0 || (r = syscall_mem_alloc(0, fd1, PTE_D | PTE_LIBRARY)) < 0) {
		goto err1;
	}

	/* Step 2: Allocate and map the page for the 'Pipe' structure. */
	va = fd2data(fd0);
	if ((r = syscall_mem_alloc(0, (void *)va, PTE_D | PTE_LIBRARY)) < 0) {
		goto err2;
	}
	if ((r = syscall_mem_map(0, (void *)va, 0, (void *)fd2data(fd1), PTE_D | PTE_LIBRARY)) <
	    0) {
		goto err3;
	}

```

设定文件描述符的相关属性，最后返回。还有异常处理部分，都不再详述。
```c
	/* Step 3: Set up 'Fd' structures. */
	fd0->fd_dev_id = devpipe.dev_id;
	fd0->fd_omode = O_RDONLY;

	fd1->fd_dev_id = devpipe.dev_id;
	fd1->fd_omode = O_WRONLY;

	debugf("[%08x] pipecreate \n", env->env_id, vpt[VPN(va)]);

	/* Step 4: Save the result. */
	pfd[0] = fd2num(fd0);
	pfd[1] = fd2num(fd1);
	return 0;

err3:
	syscall_mem_unmap(0, (void *)va);
err2:
	syscall_mem_unmap(0, fd1);
err1:
	syscall_mem_unmap(0, fd0);
err:
	return r;
}
```

管道的数据结构实际上是一个环形队列，是通过共享内存的方式实现进程间的通信。关键的内容就在于管道的读写操作。实际上对于并发的临界区资源读写，有很多需要考虑的细节，但碍于篇幅，还是不详述了。指导书上的说明已经很详尽了，内容就到此为止吧。
```c
static int _pipe_is_closed(struct Fd *fd, struct Pipe *p) {
	// The 'pageref(p)' is the total number of readers and writers.
	// The 'pageref(fd)' is the number of envs with 'fd' open
	// (readers if fd is a reader, writers if fd is a writer).
	//
	// Check if the pipe is closed using 'pageref(fd)' and 'pageref(p)'.
	// If they're the same, the pipe is closed.
	// Otherwise, the pipe isn't closed.

	int fd_ref, pipe_ref, runs;
	// Use 'pageref' to get the reference counts for 'fd' and 'p', then
	// save them to 'fd_ref' and 'pipe_ref'.
	// Keep retrying until 'env->env_runs' is unchanged before and after
	// reading the reference counts.
	/* Exercise 6.1: Your code here. (1/3) */
	do {
		runs = env->env_runs;

		fd_ref = pageref(fd);
		pipe_ref = pageref(p);

	} while (runs != env->env_runs);

	return fd_ref == pipe_ref;
}

static int pipe_read(struct Fd *fd, void *vbuf, u_int n, u_int offset) {
	int i;
	struct Pipe *p;
	char *rbuf;

	// Use 'fd2data' to get the 'Pipe' referred by 'fd'.
	// Write a loop that transfers one byte in each iteration.
	// Check if the pipe is closed by '_pipe_is_closed'.
	// When the pipe buffer is empty:
	//  - If at least 1 byte is read, or the pipe is closed, just return the number
	//    of bytes read so far.
	//  - Otherwise, keep yielding until the buffer isn't empty or the pipe is closed.
	/* Exercise 6.1: Your code here. (2/3) */
	p = fd2data(fd);
	rbuf = (char *)vbuf;
	for (i = 0; i < n; i++) {
		while (p->p_rpos >= p->p_wpos) {
			if (i > 0 || _pipe_is_closed(fd, p)) {
				return i;
			} else {
				syscall_yield();
			}
		}
		rbuf[i] = p->p_buf[p->p_rpos % BY2PIPE];
		p->p_rpos++;
	}
	return n;
}

static int pipe_write(struct Fd *fd, const void *vbuf, u_int n, u_int offset) {
	int i;
	struct Pipe *p;
	char *wbuf;

	// Use 'fd2data' to get the 'Pipe' referred by 'fd'.
	// Write a loop that transfers one byte in each iteration.
	// If the bytes of the pipe used equals to 'BY2PIPE', the pipe is regarded as full.
	// Check if the pipe is closed by '_pipe_is_closed'.
	// When the pipe buffer is full:
	//  - If the pipe is closed, just return the number of bytes written so far.
	//  - If the pipe isn't closed, keep yielding until the buffer isn't full or the
	//    pipe is closed.
	/* Exercise 6.1: Your code here. (3/3) */
	p = fd2data(fd);
	wbuf = (char *)vbuf;
	for (i = 0; i < n; i++) {
		while (p->p_wpos - p->p_rpos >= BY2PIPE) {
			if (_pipe_is_closed(fd, p)) {
				return i;
			} else {
				syscall_yield();
			}
		}
		p->p_buf[p->p_wpos % BY2PIPE] = wbuf[i];
		p->p_wpos++;
	}
	return n;
}

int pipe_is_closed(int fdnum) {
	struct Fd *fd;
	struct Pipe *p;
	int r;

	// Step 1: Get the 'fd' referred by 'fdnum'.
	if ((r = fd_lookup(fdnum, &fd)) < 0) {
		return r;
	}
	// Step 2: Get the 'Pipe' referred by 'fd'.
	p = (struct Pipe *)fd2data(fd);
	// Step 3: Use '_pipe_is_closed' to judge if the pipe is closed.
	return _pipe_is_closed(fd, p);
}
```

**（完）**
