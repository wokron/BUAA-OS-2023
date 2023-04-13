
## 一、Lab3 前言
不知道为什么，虽然写 Lab3 所用的时间比 Lab2 少，但这次的笔记居然比 Lab2 长。我认为可能是因为自己在本篇文章中讲了更多和实验本身无关的东西。不过既然讲了，应该也会对进一步认识操作系统起到一些作用吧。希望本篇文章不会显得太啰嗦。
<!-- more -->

## 二、内核初始化（再续）
Lab2 中，我们在内核初始化阶段初始化了虚拟内存的相关信息，Lab3 中我们要继续这一过程。本次实验中我们会完成进程控制的初始化。

### （1）再度 mips_init
我们查看 Lab3 中 init/init.c 的 `mips_init` 函数的内容变化。与 Lab2 相比，其中多调用了如下的方法 `env_init`、`ENV_CREATE_PRIORITY`、`kclock_init` 和 `enable_irq`。
```c

void mips_init() {
	printk("init.c:\tmips_init() is called\n");

	// lab2:
	mips_detect_memory();
	mips_vm_init();
	page_init();

	// lab3:
	env_init();

	// lab3:
	ENV_CREATE_PRIORITY(user_bare_loop, 1);
	ENV_CREATE_PRIORITY(user_bare_loop, 2);

	// lab3:
	kclock_init();
	enable_irq();
	while (1) {
	}
}
```

其中 `env_init` 用于进程控制的初始化，`ENV_CREATE_PRIORITY` 手工创建了两个进程，`kclock_init` 和 `enable_irq` 设置了时钟中断并启用了中断。后两者将分别在第三和四节介绍。本届只介绍前者。

### （2）进程管理的数据结构

让我们深入在 kern/env.c 中的 `env_init`，在该函数中，首先初始化了两个列表
```c
void env_init(void) {
	int i;
	/* Step 1: Initialize 'env_free_list' with 'LIST_INIT' and 'env_sched_list' with
	 * 'TAILQ_INIT'. */
	/* Exercise 3.1: Your code here. (1/2) */
	LIST_INIT(&env_free_list);
	TAILQ_INIT(&env_sched_list);

```

这两个列表，准确来说是一个链表和一个尾队列（此类型的定义可以在 include/queue.h 中找到），其中的元素都是同一类型。我们可以在 include/env.h 中找到
```c
LIST_HEAD(Env_list, Env);
TAILQ_HEAD(Env_sched_list, Env);
```

其中 `Env` 是如下的结构体，它被称为进程控制块（Process Control Block，PCB），其中保存了一个进程所拥有的不同的资源。或许因为是附属于进程，为进程提供支持的结构，就像是进程所处的环境一样，因此这里类型名为Env(ironment)。
```c
struct Env {
	struct Trapframe env_tf;  // Saved registers
	LIST_ENTRY(Env) env_link; // Free list
	u_int env_id;		  // Unique environment identifier
	u_int env_asid;		  // ASID
	u_int env_parent_id;	  // env_id of this env's parent
	u_int env_status;	  // Status of the environment
	Pde *env_pgdir;		  // Kernel virtual address of page dir
	TAILQ_ENTRY(Env) env_sched_link;
	u_int env_pri;
	// Lab 4 IPC
	u_int env_ipc_value;   // data value sent to us
	u_int env_ipc_from;    // envid of the sender
	u_int env_ipc_recving; // env is blocked receiving
	u_int env_ipc_dstva;   // va at which to map received page
	u_int env_ipc_perm;    // perm of page mapping received

	// Lab 4 fault handling
	u_int env_user_tlb_mod_entry; // user tlb mod handler

	// Lab 6 scheduler counts
	u_int env_runs; // number of times been env_run'ed
};
```

进程控制块中内容众多，不好一一解释。因此这里只稍微介绍一下和本次实验有关的字段。
- `env_link`、`env_sched_link`：根据类型就可以看出，这两个字段就是存储链表信息的字段
- `env_tf`：此字段用于在陷入内核时保存当前进程所处状态的相关信息，比如寄存器的值、pc寄存器中的地址等等。它的类型名为 `Trapframe`，与栈帧（stack frame）类似，都是存储一段信息的结构，只不过此类型存储的是发生在陷入内核（trap）时的信息罢了。`Trapframe` 定义在 include/trap.h 中
    ```c
    struct Trapframe {
        /* Saved main processor registers. */
        unsigned long regs[32];

        /* Saved special registers. */
        unsigned long cp0_status;
        unsigned long hi;
        unsigned long lo;
        unsigned long cp0_badvaddr;
        unsigned long cp0_cause;
        unsigned long cp0_epc;
    };
    ```
- `env_id`：此字段是进程的标识符，每个进程都唯一
- `env_status`：此字段中存储进程的当前状态，包括空闲、阻塞和可运行
    ```c
    #define ENV_FREE 0
    #define ENV_RUNNABLE 1
    #define ENV_NOT_RUNNABLE 2
    ```
- `env_asid`：表示进程的 asid，用于 tlb 中
- `env_pgdir`：存储了当前进程拥有的页目录的虚拟地址
- `env_pri`：表示当前进程的优先级

在本实验接下来的代码中，我们都会使用或看到这些字段。

现在让我们回到 `env_init`。看一下我们初始化的两个数据结构的名称。`env_free_list` 表示其中存储了所有空闲的进程控制块，`env_sched_list` 则意味着该列表用于组织进程的调度（schedule）。

### （3）map_segment 函数
我们查看 `env_init` 的后续内容。首先，我们将所有的进程控制块都插入 `env_free_list` 中。并且标记所有块都为 `ENV_FREE`。这里插入时顺序反向，这只是指导书的要求而已。
```c
	for (i = NENV - 1; i >= 0; i--) {
		LIST_INSERT_HEAD(&env_free_list, envs + i, env_link);
		envs[i].env_status = ENV_FREE;
	}
```

在 `env_init` 中我们还需要做一件事，就是创建一个 “模板页目录”，设置该页将 pages 和 envs （即所有页控制块和所有进程控制块的内存空间）分别映射到 `UPAGES` 和 `UENVS` 的空间中。并且在后续进程创建新的页目录时，也要首先复制模板页目录中的内容。这样做的目的是使得用户程序也能够通过 `UPAGES` 和 `UENVS` 的用户地址空间获取 `Page` 和 `Env` 的信息。

我们首先调用 `page_alloc` 申请一个页
```c
	struct Page *p;
	panic_on(page_alloc(&p));
	p->pp_ref++;
```

该页即 “模板页目录”，我们把它的地址存储到全局变量 `base_pgdir` 中
```c
	base_pgdir = (Pde *)page2kva(p);
```

最后我们调用 `map_segment` 函数，该函数在指定的页目录中创建虚拟地址空间 `[va, va+size)` 到物理地址空间 `[pa, pa+size)` 的映射。并设置其权限（在这里我们设置其为只读，因为不希望用户程序修改内核空间的内容）。
```c
	map_segment(base_pgdir, 0, PADDR(pages), UPAGES, ROUND(npage * sizeof(struct Page), BY2PG),
		    PTE_G);
	map_segment(base_pgdir, 0, PADDR(envs), UENVS, ROUND(NENV * sizeof(struct Env), BY2PG),
		    PTE_G);
}
```

这样 `env_init` 函数就结束了。我们初始化了两个列表；将所有进程控制块插入空闲进程表；创建了一个 “模板页目录” 并将一部分内核空间的内容映射到用户空间。

接着我们考察 `map_segment` 函数。可以认为该函数是一个广义的 `page_insert`。它的功能是，在页目录 `pgdir` 中，将虚拟地址空间 `[va, va+size)` 映射到到物理地址空间 `[pa, pa+size)`，并赋予 `perm` 权限。
```c
static void map_segment(Pde *pgdir, u_int asid, u_long pa, u_long va, u_int size, u_int perm) {
```

它的实现也很简单，就是通过循环不断调用 `page_insert` 创建一个页大小的 `va` 到 `pa` 的映射，直到达到期望的 `size` 大小
```c
	for (int i = 0; i < size; i += BY2PG) {
		/*
		 * Hint:
		 *  Map the virtual page 'va + i' to the physical page 'pa + i' using 'page_insert'.
		 *  Use 'pa2page' to get the 'struct Page *' of the physical address.
		 */
		/* Exercise 3.2: Your code here. */
		page_insert(pgdir, asid, pa2page(pa + i), va + i, perm);
	}
}
```

这样进程控制的初始化就完成了。

## 三、进程的创建
### （1）再再度 mips_init
我们再回到 `mips_init` 函数。进程控制初始化完成后，我们又调用 `ENV_CREATE_PRIORITY` 宏创建了两个进程。
```c
	// lab3:
	ENV_CREATE_PRIORITY(user_bare_loop, 1);
	ENV_CREATE_PRIORITY(user_bare_loop, 2);
```

该宏定义在 include/env.h 中。在同一个文件夹中还有另一个类似的宏 `ENV_CREATE`，相当于把 `ENV_CREATE_PRIORITY` 中的 `y` 设为 1。
```c
#define ENV_CREATE_PRIORITY(x, y) \
	({ \
		extern u_char binary_##x##_start[]; \
		extern u_int binary_##x##_size; \
		env_create(binary_##x##_start, (u_int)binary_##x##_size, y); \
	})

```

我们可以看出宏中定义了两个外部引用变量 `binary_##x##_start` 和 `binary_##x##_size`，对于 `mips_init` 中的使用来说即 `binary_user_bare_loop_start` 和 `binary_user_bare_loop_size`。接着我们调用了 `env_create` 函数。很明显该函数用于创建一个进程。

### （2）一种神奇的操作
`env_create` 定义在 kern/env.c 中。在介绍该函数的内容之前，我们可以解释一下该函数的参数。
```c
struct Env *env_create(const void *binary, size_t size, int priority) {
```

首先，`const void *binary` 是一个二进制的数据数组，该数组的大小为 `size_t size`，实际上此二进制数据即我们想要创建的进程的程序。最后一个参数 `int priority` 表示我们想要设置的进程的优先级，对应 `Env` 中的 `env_pri` 字段。

你可能会想，“不对呀，我们从哪里读入的程序？” 确实，我们根本没有进行磁盘操作。我们还没有实现文件系统，我们所 “加载” 的程序实际上是被一同编译到内核中的一段 ELF 格式的数据。这段数据中存在标签 `binary_user_bare_loop_start` 和 `binary_user_bare_loop_size`，所以我们才可以只通过引用外部变量的形式就 “加载” 了程序文件。

将 ELF 文件转化为 c 数组一同编译进内核程序的过程似乎较为复杂。本人也并不像如此深入此与操作系统无关的技术。但是经过简单的探索，我们还是可以了解到实现这一过程的程序的源代码为 tools/bintoc.c。此代码实现了一个程序，可以读取某一 ELF 文件的二进制内容，将其转化为一个 c 语言源代码文件。bintoc.c 的代码片段如下：
```c
    size_t n = fread(binary, sizeof(char), size, bin);
	assert(n == size);
	fprintf(out,
		"unsigned int binary_%s_%s_size = %d;\n"
		"unsigned char binary_%s_%s_start[] = {",
		prefix, bin_file, size, prefix, bin_file);
	for (i = 0; i < size; i++) {
		fprintf(out, "0x%x%c", binary[i], i < size - 1 ? ',' : '}');
	}
```

另外，我们可以在 user/bar/Makefile 中找到如下内容
```makefile
INCLUDES    := -I../../include

%.b.c: %.b
	$(tools_dir)/bintoc -f $< -o $@ -p user_bare
```

这些内容就足以使人察觉 “将 ELF 文件转化为 c 数组一同编译进内核程序” 的方法了。

### （3）进入 env_alloc 函数
说了这么多没用的，我们还是回到 `env_create` 吧。首先，该函数通过调用 `env_alloc` 申请了一个新的空闲进程控制块。
```c
	struct Env *e;
	/* Step 1: Use 'env_alloc' to alloc a new env. */
	/* Exercise 3.7: Your code here. (1/3) */
	env_alloc(&e, 0);
```

`env_alloc` 与 `page_alloc` 类似，都从空闲块列表中取出一个空闲块。但是 `env_alloc` 的后续处理更加复杂。首先 `env_alloc` 中同样取出一个空闲块。
```c
int env_alloc(struct Env **new, u_int parent_id) {
	int r;
	struct Env *e;

	/* Step 1: Get a free Env from 'env_free_list' */
	/* Exercise 3.4: Your code here. (1/4) */
	if (LIST_EMPTY(&env_free_list)) {
		return -E_NO_FREE_ENV;
	}
	e = LIST_FIRST(&env_free_list);
```

接着我们调用 `env_setup_vm` 初始化进程控制块的用户地址空间。也就是为进程控制块创建对应的二级页表。
```c
	/* Step 2: Call a 'env_setup_vm' to initialize the user address space for this new Env. */
	/* Exercise 3.4: Your code here. (2/4) */
	if ((r = env_setup_vm(e)) != 0) {
		return r;
	}
```

### （4）env_setup_vm 与页表自映射
`env_setup_vm` 函数值得关注一下，我们查看一下该函数的内容。首先我们申请一个物理页作为页目录。可以看到我们这里设置了 `Env` 中 `env_pgdir` 字段的值
```c
static int env_setup_vm(struct Env *e) {
	struct Page *p;
	try(page_alloc(&p));
	/* Exercise 3.3: Your code here. */
	p->pp_ref++;
	e->env_pgdir = (Pde *)page2kva(p);
```

值得讲解的在后面。是否还记得我们之前花大篇幅讲解的 “模板页目录”？现在正在创建二级页表，我们可以将 “模板页目录” 中的内容复制到当前进程的页目录中。我们复制了 `UTOP` 到 `UVPT` 的虚拟地址空间对应的页表项。这就是我们之前在 “模板页目录” 中映射的区域。
```c
	memcpy(e->env_pgdir + PDX(UTOP), base_pgdir + PDX(UTOP),
	       sizeof(Pde) * (PDX(UVPT) - PDX(UTOP)));
```

`env_setup_vm` 函数还没有结束。在最后我们还执行了这样的语句
```c
	e->env_pgdir[PDX(UVPT)] = PADDR(e->env_pgdir) | PTE_V;
	return 0;
}
```

我们将 `UVPT` 虚拟地址映射到页目录本身的物理地址，并设置只读权限。这样的话，页目录中的项所对应的，就不只是二级页表，还包含有一个一级页表，也就是页目录自身。这就是自映射。我们在 Lab2 时从指导书上学到了关于自映射的理论。但到了 Lab3 才把它用代码表示了出来。

所以，自映射有什么用？指导书上的表达有些模糊不清，虽然我也没有信心能够表述明白，但我还是试一下吧。

>假设现在我们用比 `UVPT` 地址高一些的地址 `va` 进行访存，那么我们会取到那些信息呢？首先，这个地址会经过页目录，`PDX(va)` 的结果和 `UVPT` 相同，我们进入到索引对应的二级页表……不对，还是页目录自身！
>
>好的，我们在页目录中重新来一遍，这次通过 `PTX(va)` 计算索引，结果就不一定还是页目录项了。我们找到了一个物理页，取出了其中的数据。可是等等，这个物理页却不再是一般的物理页了，而是作为二级页表的物理页。
>
>另外假如我们恰好取得的 `PTX(va)` 值与 `PDX(va)` 相同，那么我们绕了两圈，最终还是处在页目录之中，我们取得的数据也是页目录中的内容。
>
>这样我们就可以明白自映射的作用了。它在用户内存空间中划分出一部分，使得用户可以通过访问这部分空间得到二级页表以及页目录中的数据。
>
>在 include/mmu.h 的内存分布图中，我们可以看出 `UVPT` 以上的 4kb（1024 个页表的大小）空间被标记为 `User VPT`。VPT 或为 virtual page table（虚拟页表）的意思。

### （5）进程控制块的初始化
我们回到 `env_alloc` 函数。此后的内容是初始化新申请的进程控制块。我们要设置一些字段的值后才算完成了进程控制块的申请。

一些内容和后续的实验有关，我们先不考虑，我们考虑本次试验中涉及的部分。这里我们设置了 `env_id`、`env_parent_id` 和 `env_asid` 的值。其中 `env_parent_id` 设置为 `env_alloc` 的参数。而 `env_id` 和 `env_asid` 因为需要不重复，所以通过两个函数分别申请。
```c
	e->env_user_tlb_mod_entry = 0; // for lab4
	e->env_runs = 0;	       // for lab6
	/* Exercise 3.4: Your code here. (3/4) */
	e->env_id = mkenvid(e);
	e->env_parent_id = parent_id;
	if ((r = asid_alloc(&e->env_asid)) != 0) {
		return r;
	}
```

`mkenvid` 的内容较为简单，只是通过一个函数内的静态变量实现不重复的。
```c
u_int mkenvid(struct Env *e) {
	static u_int i = 0;
	return ((++i) << (1 + LOG2NENV)) | (e - envs);
}
```

而 `asid_alloc` 则是设置了一个 `asid_bitmap` 用来管理 asid 的分配情况，这是因为 asid 是有限的（2^6=64）。当所有的 asid 都被分配以后，应该返回异常。
```c
static uint32_t asid_bitmap[NASID / 32] = {0}; // 64

/* Overview:
 *  Allocate an unused ASID.
 *
 * Post-Condition:
 *   return 0 and set '*asid' to the allocated ASID on success.
 *   return -E_NO_FREE_ENV if no ASID is available.
 */
static int asid_alloc(u_int *asid) {
	for (u_int i = 0; i < NASID; ++i) {
		int index = i >> 5;
		int inner = i & 31;
		if ((asid_bitmap[index] & (1 << inner)) == 0) {
			asid_bitmap[index] |= 1 << inner;
			*asid = i;
			return 0;
		}
	}
	return -E_NO_FREE_ENV;
}
```

继续 `env_alloc` 函数，在最后，我们设置进程的 `status` 寄存器和 `sp` 寄存器的值。
```c
	/* Step 4: Initialize the sp and 'cp0_status' in 'e->env_tf'. */
	// Timer interrupt (STATUS_IM4) will be enabled.
	e->env_tf.cp0_status = STATUS_IM4 | STATUS_KUp | STATUS_IEp;
	// Keep space for 'argc' and 'argv'.
	e->env_tf.regs[29] = USTACKTOP - sizeof(int) - sizeof(char **);

	/* Step 5: Remove the new Env from env_free_list. */
	/* Exercise 3.4: Your code here. (4/4) */
	LIST_REMOVE(e, env_link);

	*new = e;
	return 0;
}
```

将`status` 寄存器的值设置为 `STATUS_IM4 | STATUS_KUp | STATUS_IEp`，表示响应 4 号中断，是用户状态且开启中断。所有的通用寄存器状态在 `Trapframe` 中存储在 `regs` 数组中，其中第 29 号寄存器为 `sp` 寄存器。这一点可从 include/asm/regdef.h 中得知
```c
#define sp $29 /* stack pointer */
```

在未执行的情况下，用户程序的 sp 寄存器应该处于栈顶 `USTACKTOP` 的位置。但为了给程序的 `main` 函数的参数 `argc` 和 `argv` 留出空间，需要减去 `sizeof(int) + sizeof(char **)` 的大小。

这样我们就完成了对 `env_alloc` 函数的讲解。

### （6）加载 ELF 文件
经历了这么多，我们都快忘记最开始我们的目标，`env_create` 函数了。让我们接着 `env_alloc` 之后的内容。现在对于调用 `env_alloc` 得到的新进程控制块，我们设置它的优先级以及状态。
```c
	/* Step 2: Assign the 'priority' to 'e' and mark its 'env_status' as runnable. */
	/* Exercise 3.7: Your code here. (2/3) */
	e->env_pri = priority;
	e->env_status = ENV_RUNNABLE;
```

最后，我们调用 `load_icode` 为进程加载 ELF 程序，同时使用 `TAILQ_INSERT_HEAD` 宏将进程控制块加入到调度队列中
```c
	/* Step 3: Use 'load_icode' to load the image from 'binary', and insert 'e' into
	 * 'env_sched_list' using 'TAILQ_INSERT_HEAD'. */
	/* Exercise 3.7: Your code here. (3/3) */
	load_icode(e, binary, size);
	TAILQ_INSERT_HEAD(&env_sched_list, e, env_sched_link);

	return e;
}
```

我们就差最后一步就完成了 “进程的创建” 这一节的内容了，现在我们要分析 `load_icode` 中加载二进制镜像功能的实现。函数名中 icode 似乎指 image code 的意思。`load_icode` 同样在 kern/env.c 中。

首先该函数调用了 `elf_from` 函数从二进制数据中读取了页表信息。
```c
static void load_icode(struct Env *e, const void *binary, size_t size) {
	/* Step 1: Use 'elf_from' to parse an ELF header from 'binary'. */
	const Elf32_Ehdr *ehdr = elf_from(binary, size);
	if (!ehdr) {
		panic("bad elf at %x", binary);
	}
```

`elf_from` 函数定义在 lib/elfloader.c 中。只是简单地对二进制数据做类型转换，并检查是否确为 ELF 文件头。
```c
const Elf32_Ehdr *elf_from(const void *binary, size_t size) {
	const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)binary;
	if (size >= sizeof(Elf32_Ehdr) && ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
	    ehdr->e_ident[EI_MAG1] == ELFMAG1 && ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
	    ehdr->e_ident[EI_MAG3] == ELFMAG3 && ehdr->e_type == 2) {
		return ehdr;
	}
	return NULL;
}
```

接着在 `load_icode` 中使用了一个宏 `ELF_FOREACH_PHDR_OFF` 来遍历所有的程序头表
```c
	/* Step 2: Load the segments using 'ELF_FOREACH_PHDR_OFF' and 'elf_load_seg'.
	 * As a loader, we just care about loadable segments, so parse only program headers here.
	 */
	size_t ph_off;
	ELF_FOREACH_PHDR_OFF (ph_off, ehdr) {
```

这个宏定义在 include/elf.h 中。很容易看出作用
```c
#define ELF_FOREACH_PHDR_OFF(ph_off, ehdr) \
	(ph_off) = (ehdr)->e_phoff; \
	for (int _ph_idx = 0; _ph_idx < (ehdr)->e_phnum; ++_ph_idx, (ph_off) += (ehdr)->e_phentsize)
```

在循环中，取出对应的程序头，如果其中的 `p_type` 类型为 `PT_LOAD`，说明其对应的程序需要被加载到内存中。我们调用 `elf_load_seg` 函数来进行加载
```c
		Elf32_Phdr *ph = (Elf32_Phdr *)(binary + ph_off);
		if (ph->p_type == PT_LOAD) {
			// 'elf_load_seg' is defined in lib/elfloader.c
			// 'load_icode_mapper' defines the way in which a page in this segment
			// should be mapped.
			panic_on(elf_load_seg(ph, binary + ph->p_offset, load_icode_mapper, e));
		}
	}
```

在 `load_icode` 函数的最后，我们将进程控制块中 trap frame 的 epc cp0 寄存器的值设置为 ELF 文件中设定的程序入口地址
```c
	/* Step 3: Set 'e->env_tf.cp0_epc' to 'ehdr->e_entry'. */
	/* Exercise 3.6: Your code here. */
	e->env_tf.cp0_epc = ehdr->e_entry;
}
```

这样 `load_icode` 部分也完成了。让我们回过头查看一下 `elf_load_seg` 函数。该函数定义在 lib/elfloader.c 中。作用是根据程序头表中的信息将 `bin` 中的数据加载到指定位置。值得关注该函数的参数，`elf_mapper_t map_page` 是一个回调函数，用于将数据映射到虚拟地址所在的页上；`void *data` 则是回调函数中使用的参数。
```c
int elf_load_seg(Elf32_Phdr *ph, const void *bin, elf_mapper_t map_page, void *data) {
	u_long va = ph->p_vaddr;
	size_t bin_size = ph->p_filesz;
	size_t sgsize = ph->p_memsz;
	u_int perm = PTE_V;
	if (ph->p_flags & PF_W) {
		perm |= PTE_D;
	}
```

`elf_mapper_t` 定义在 include/elf.h 中。此类型的函数接受数据要加载到的虚拟地址 `va`，数据加载的起始位置相对于页的偏移 `offset`，页的权限 `prem`，所要加载的数据 `src` 和要加载的数据大小 `len`。当然还有 `data`，但这个让我们留到后面。
```c
typedef int (*elf_mapper_t)(void *data, u_long va, size_t offset, u_int perm, const void *src,
			    size_t len);
```

在 `elf_load_seg` 中，我们首先需要处理要加载的虚拟地址不与页对齐的情况。我们将最开头不对齐的部分 “剪切” 下来，先映射到内存的页中。
```c
	int r;
	size_t i;
	u_long offset = va - ROUNDDOWN(va, BY2PG);
	if (offset != 0) {
		if ((r = map_page(data, va, offset, perm, bin, MIN(bin_size, BY2PG - offset))) !=
		    0) {
			return r;
		}
	}
```

接着我们处理数据中间完整的部分。我们通过循环不断将数据加载到页上。
```c
	/* Step 1: load all content of bin into memory. */
	for (i = offset ? MIN(bin_size, BY2PG - offset) : 0; i < bin_size; i += BY2PG) {
		if ((r = map_page(data, va + i, 0, perm, bin + i, MIN(bin_size - i, BY2PG))) != 0) {
			return r;
		}
	}
```

最后我们处理段大小大于数据大小的情况。在这一部分，我们不断创建新的页，但是并不向其中加载任何内容。
```c
	/* Step 2: alloc pages to reach `sgsize` when `bin_size` < `sgsize`. */
	while (i < sgsize) {
		if ((r = map_page(data, va + i, 0, perm, NULL, MIN(bin_size - i, BY2PG))) != 0) {
			return r;
		}
		i += BY2PG;
	}
	return 0;
}
```

这节的最最后，我们查看一下在本次实验中使用的回调函数 `load_icode_mapper`。根据 `load_icode` 中 `elf_load_seg` 传入的参数可知，此时我们的 `data` 为要加载程序镜像的进程对应的进程控制块。
```c
panic_on(elf_load_seg(ph, binary + ph->p_offset, load_icode_mapper, e));
```

`load_icode_mapper` 定义在 kern/env.c 中。在函数的一开始，我们就将 `data` 还原为进程控制块
```c
static int load_icode_mapper(void *data, u_long va, size_t offset, u_int perm, const void *src,
			     size_t len) {
	struct Env *env = (struct Env *)data;
	struct Page *p;
	int r;
```

我们想要将数据加载到内存，首先需要申请物理页。调用 `page_alloc` 函数申请空闲页
```c
	/* Step 1: Allocate a page with 'page_alloc'. */
	/* Exercise 3.5: Your code here. (1/2) */
	if ((r = page_alloc(&p)) != 0) {
		return r;
	}
```

接着，如果存在需要拷贝的数据，则将该数据复制到新申请的页所对应的内存空间中。我们使用 `page2kva` 获取页所对应的内核虚拟地址。另外注意这里需要考虑 `offset`。
```c
	/* Step 2: If 'src' is not NULL, copy the 'len' bytes started at 'src' into 'offset' at this
	 * page. */
	// Hint: You may want to use 'memcpy'.
	if (src != NULL) {
		/* Exercise 3.5: Your code here. (2/2) */
		memcpy((void *)(page2kva(p) + offset), src, len);
	}
```

最后我们调用 `page_insert` 将虚拟地址映射到页上。为了区别不同进程的相同虚拟地址，我们需要附加 asid 信息，asid 保存在进程控制块中，这也是我们需要将进程控制块传入回调函数的原因。
```c
	/* Step 3: Insert 'p' into 'env->env_pgdir' at 'va' with 'perm'. */
	return page_insert(env->env_pgdir, env->env_asid, p, va, perm);
}
```

到此为止，我们终于完成了进程创建的流程。在这一过程中，我们申请了新的进程控制块，初始化了该控制块的虚拟内存管理机制以及 trap frame 等其他信息。并将程序镜像加载到了该进程独占的虚拟内存空间中。

但是到目前为止，我们的进程还未运行起来，还不是动态的程序；仅是在内存空间中的一些有组织的数据而已。在接下来的小节中，我们会让进程运行起来。

## 四、异常处理
### （1）中断的初始化
让我们在这一次实验中最后一次查看一下 `mips_init` 函数。在该函数的最后是一个死循环。这样的程序要如何退出呢？看似在这种情况下，其他程序都不能执行。这似乎是正确的，如果在没有开启中断的情况下。

然而，我们在死循环之前调用了两个函数 `kclock_init`，`enable_irq`。这两个函数是跳出死循环，实现进程运行的关键。

让我们深入 `kclock_init`，这是一个用汇编编写的函数，定义在 kern/kclock.S 中。该函数的作用是启用时钟。使其以 200Hz 的频率触发时钟中断。
```c
LEAF(kclock_init)
	li      t0, 200 // the timer interrupt frequency in Hz

	/* Write 't0' into the timer (RTC) frequency register.
	 *
	 * Hint:
	 *   You may want to use 'sw' instruction and constants 'DEV_RTC_ADDRESS' and
	 *   'DEV_RTC_HZ' defined in include/drivers/dev_rtc.h.
	 *   To access device through mmio, a physical address must be converted to a
	 *   kseg1 address.
	 *
	 * Reference: http://gavare.se/gxemul/gxemul-stable/doc/experiments.html#expdevices
	 */
	/* Exercise 3.11: Your code here. */
	sw	t0, (KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_HZ)

	jr      ra
END(kclock_init)
```

这个函数的内容很简单，只是将数值 200 存入内存中的某一地址。地址 `KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_HZ` 的形式我们已经在 Lab1 和 Lab2 遇到过类似的了。唯一需要注意的是这里汇编的写法，直接将一个表达式作为汇编指令的参数。如果事先未见过这种写法似乎很难想到。

但是现在 cpu 依旧无法收到时钟异常。如果还记得 Lab1 中关于 `_start` 函数的内容，应该可以记得我们在最开始便屏蔽了中断
```c
EXPORT(_start)
.set at
.set reorder
	/* disable interrupts */
	mtc0    zero, CP0_STATUS
```

现在我们要中断使能，实现该功能的函数即 `enable_irq`，位于 kern/env_asm.S 中。irq 为 interrupt request 的缩写。
```c
LEAF(enable_irq)
	li      t0, (STATUS_CU0 | STATUS_IM4 | STATUS_IEc)
	mtc0    t0, CP0_STATUS
	jr      ra
END(enable_irq)
```

可以看到该函数的内容与 `_start` 中的指令相对应。这里我们为 cp0 寄存器 `status` 设置了值 `STATUS_CU0 | STATUS_IM4 | STATUS_IEc`。如果对 `status` 寄存器还有印象，应该能记得在 “进程控制块的初始化” 一节中我们设置了 trap frame 中的 `status` 寄存器的值。那时我们似乎已经设置了 4 号中断（4 号中断就是时钟中断），也使能了中断，为何现在还要设置？

因为那时我们只是设置了进程开始运行时，`status` 的状态；而还未设置当前的内核初始化环境中的状态。当系统切换到用户态，运行进程的时候，我们会用进程的状态覆盖之前的状态。现在设置 `status` 寄存器后，我们才能通过时钟中断进行上下文切换。也就是说，`enable_irq` 只是开启了内核初始化程序的中断，而开启中断的唯一目的就是通过时钟中断切换到第一个进程。

另外请注意，在之前我们设置的值为 `STATUS_IEp`，而此处为 `STATUS_IEc`，为什么两者位置却同样表示中断使能？实际上这来自于用户态和内核态的区别，具体可见实验指导书。

### （2）异常处理流程
中断是一种异常，当产生时钟中断时，cpu 就将执行异常处理流程。具体来说，当异常产生时，cpu 就会自动跳转到虚拟地址 `0x80000080` 处（特别的，当在用户态产生 TLB miss 异常时，会跳转到 `0x80000000`），从此处执行程序。这一程序应该完成异常的处理，并使 cpu 返回正常程序。

对于 MOS 来说，此处实现了一个异常分发函数，根据异常的不同类型选择不同的异常处理函数。此函数在 init/entry.S 中。
```c
.section .text.tlb_miss_entry
tlb_miss_entry:
	j       exc_gen_entry

.section .text.exc_gen_entry
exc_gen_entry:
	SAVE_ALL
/* Exercise 3.9: Your code here. */
	mfc0 	t0, CP0_CAUSE
	andi 	t0, 0x7c
	lw 	t0, exception_handlers(t0)
	jr 	t0
```

`tlb_miss_entry` 用于处理 TLB miss 异常，但实际上就是跳转到 `exc_gen_entry`。而在 `exc_gen_entry` 中，我们首先使用了一个宏 `SAVE_ALL`，该宏定义了一大段指令，用于将所有的寄存器值存储到栈帧中。这样我们便保存了异常发生时的上下文。唯一需要注意的是，在 `SAVE_ALL` 中，我们将栈帧的初始位置设置为 `KSTACKTOP`（之前的 `sp` 位置保存在 `TF_REG29(sp)`）。
```c
// clang-format off
.macro SAVE_ALL
.set noreorder
.set noat
	move    k0, sp
.set reorder
	bltz    sp, 1f
	li      sp, KSTACKTOP
.set noreorder
1:
	subu    sp, sp, TF_SIZE
	sw      k0, TF_REG29(sp)
	mfc0    k0, CP0_STATUS
	sw      k0, TF_STATUS(sp)
// omit...
	sw      $0, TF_REG0(sp)
	sw      $1, TF_REG1(sp)
	sw      $2, TF_REG2(sp)
	sw      $3, TF_REG3(sp)
// omit...
```

接下来我们获取 `cause` 寄存器的值，取其 2-6 位，这部分对应异常码，用于区别不同的异常。
```c
	mfc0 	t0, CP0_CAUSE
	andi 	t0, 0x7c
```

接下来的这一部分，我们从 `exception_handlers` 数组中取出异常码对应的处理函数，并跳转到该异常处理函数。
```c
	lw 	t0, exception_handlers(t0)
	jr 	t0
```

其中 `exception_handlers` 定义在 kern/traps.c 中。该数组是一个函数数组，其中每个元素都是异常码对应的异常处理函数。此数组称为异常向量组。
```c
extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_reserved(void);

void (*exception_handlers[32])(void) = {
    [0 ... 31] = handle_reserved,
    [0] = handle_int,
    [2 ... 3] = handle_tlb,
#if !defined(LAB) || LAB >= 4
    [1] = handle_mod,
    [8] = handle_sys,
#endif
};
```

需要注意的有两点。
- 第一是此数组的定义似乎语法很奇怪。此语法是 GNU C 的扩展语法，`[first ... last] = value` 用于对数组上某个区间上元素赋同一个值。
- 第二是 `exc_gen_entry` 中我们直接将 `andi 	t0, 0x7c` 的结果作为索引。这里需要注意一个地址 4 字节。

最后，`tlb_miss_entry` 和 `exc_gen_entry` 还未被放在 `0x80000000` 和 `0x80000080` 处。我们需要在 kernel.lds 中添加内容，将这两个标签固定在特定的地址位置。
```c
SECTIONS {
	/* Exercise 3.10: Your code here. */
	. = 0x80000000;
	.tlb_miss_entry : {
		*(.text.tlb_miss_entry)
	}

	. = 0x80000080;
	.exc_gen_entry : {
		*(.text.exc_gen_entry)
	}

	// omit...
}
```

### （3）异常处理函数在哪里定义？
如果你想了解一下不同异常的异常处理函数，可能会发现自己根本找不到 `handle_tlb`、`handle_mod` 等函数的定义。实际上这些函数都定义在 kern/genex.S 中。

当然，`handle_int` 的定义我们可以很直接地找到，此函数与中断有关，因此我们放到后面。

请关注位于该文件开头的宏 `BUILD_HANDLER`，构建处理函数。我们可以看到该宏有两个参数，`exception` 和 `handler`。在该宏中，我们定义了一个 `handle_\exception` 的函数，该函数调用 `\handler` 函数。返回后再调用 `ret_from_exception`（并且不返回？！）。
```c
.macro BUILD_HANDLER exception handler
NESTED(handle_\exception, TF_SIZE, zero)
	move    a0, sp
	jal     \handler
	j       ret_from_exception
END(handle_\exception)
.endm
```

让我们先不关注对 `ret_from_exception` 的调用。通过查看该宏的定义，应该可以理解 `handle_tlb` 是在哪里定义的了。我们可以在 genex.S 的最后看到如下语句。这就是异常处理函数的定义。
```c
BUILD_HANDLER tlb do_tlb_refill

#if !defined(LAB) || LAB >= 4
BUILD_HANDLER mod do_tlb_mod
BUILD_HANDLER sys do_syscall
#endif

BUILD_HANDLER reserved do_reserved
```

我们可以看到一个似曾相识的名字 `do_tlb_refill`。这个函数可以说是 Lab2 的核心。在 Lab2 的测试中我们只是模拟 tlb 的重填，而在 Lab3 中，我们终于将该函数实际应用了。

其他的异常处理函数与本次实验无关，因此在本篇文章中就不考虑了。

### （4）ret_from_exception 函数
当异常处理完成后，我们便希望能返回到正常的程序中。`ret_from_exception` 便用于从异常处理程序中返回，除了使用 `BUILD_HANDLER` 创建的处理函数，`handle_int` 的处理过程中也使用了 `ret_from_exception`，这我们留到下一节介绍。

现在我们分析 `ret_from_exception` 函数。该函数定义在 kern/genex.S 中。首先该函数调用了一个宏 `RESTORE_SOME`，用于还原栈帧中通过调用 `SAVE_ALL` 保存的（部分）上下文。该宏和 `SAVE_ALL` 一样定义在 include/stackframe.h 中，不再详细介绍。
```c
FEXPORT(ret_from_exception)
	RESTORE_SOME
```

接着将 epc 寄存器的值加载到 k0 寄存器，epc 寄存器中存储有异常处理结束后的返回地址（各位应该还对 `TrapFrame` 结构体中的 `cp0_epc` 字段有印象）；随后将所有栈帧中关于上下文的内容弹出。`TF_REG29(sp)` 地址中保存了 sp 寄存器在调用 `SAVE_ALL` 之前的地址。
```c
	lw      k0, TF_EPC(sp)
	lw      sp, TF_REG29(sp) /* Deallocate stack */
```

最后，跳转到 k0 中的返回地址。但在此之后还有另一条指令 `rfe`，用来从异常中恢复（恢复 `status` 寄存器，从内核态恢复到用户态）。
```c
.set noreorder
	jr      k0
	rfe
.set reorder
```

这样关于异常处理的部分我们就介绍完成了。

## 五、进程的调度
### （1）从 handle_int 函数继续
在操作系统中，使用时钟来划分时间片。当时钟中断发生时，就需要进行进程调度。在上一节中我们分析了 MOS 中的异常处理原理。在这一节中我们会从 `handle_int` 函数继续，讨论进程的调度机制的实现。

我们查看 `handle_int` 函数。首先经过一系列运算，从 `status` 寄存器中获得了 IM4 的值。正如前面提到过的，此值表示是否开启 4 号中断。
```c
NESTED(handle_int, TF_SIZE, zero)
	mfc0    t0, CP0_CAUSE
	mfc0    t2, CP0_STATUS
	and     t0, t2
	andi    t1, t0, STATUS_IM4
```

那么，如果是 4 号中断，也就是时钟中断，就转到时钟中断的处理。
```c
	bnez    t1, timer_irq
	// TODO: handle other irqs
```

在对时钟中断的处理中，我们首先将 `KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_INTERRUPT_ACK` 地址的值置零。此地址中数值存储了本次中断的相关信息。清零说明我们已经完成了对中断的处理。随后我们调用 `schedule` 函数，进行进程的调度。
```c
timer_irq:
	sw      zero, (KSEG1 | DEV_RTC_ADDRESS | DEV_RTC_INTERRUPT_ACK)
	li      a0, 0
	j       schedule
END(handle_int)
```

### （2）调度方法
`schedule` 函数位于 kern/sched.c 中。它有一个参数 `int yield`，用于表示是否强制让出当前进程的运行，了解过 java 多线程的应该对 yield 这个词有所认识。接下来就让我们深入这个函数。

首先，`schedule` 函数中存在一个静态变量 `count`，用于表示当前进程剩余的时间片。而 `curenv` 则是一个 `Env *` 类型的全局变量，用于表示当前运行的进程。
```c
void schedule(int yield) {
	static int count = 0; // remaining time slices of current env
	struct Env *e = curenv;
```

接着就是进程调度方法的主体，这也是本次实验我们需要填写的部分。实际上，注释已经将如何实现写得很清楚了。我们现在就分析一下该方法的原理。

首先我们考虑需要进行进程切换的情况，这在注释中有说明。
- yield 为真时：此时当前进程必须让出
- count 减为 0 时：此时分给进程的时间片被用完，将执行权让给其他进程
- 无当前进程：这必然是内核刚刚完成初始化，第一次产生时钟中断的情况，需要分配一个进程执行
- 进程状态不是可运行：当前进程不能再继续执行，让给其他进程
```c
	if (yield || count <= 0 || e == NULL || e->env_status != ENV_RUNNABLE) {
```

首先我们考虑发生进程切换的情况。我们需要从进程调度队列中取出头部的进程控制块。这时我们需要判断一些情况。当之前的进程还是可运行的时，我们需要将其插入调度队列队尾，等待下一次轮到其执行，注意在此之前需要判断 `e` 非空。而当调度队列为空时，内核崩溃，因为操作系统中必须至少有一个进程。

这里需要着重注意的是，`env_sched_list` 要存储所有状态为 `ENV_RUNNABLE` 的进程控制块，这也包括当前正在运行的进程控制块 `curenv`。这样做主要是因为在 `env_free` 函数（这个函数在 Lab4 中介绍）和之后的实验中我们预设所有状态为 `ENV_RUNNABLE` 的进程控制块都在 `env_sched_list` 中，这样就可以毫无顾忌地使用类似 `TAILQ_REMOVE(&env_sched_list, (e), env_sched_link)` 的语句了。而假若对不在队列中的元素调用 `TAILQ_REMOVE`，则可能发生异常情况，使得列表中其他元素被一并删除。

```c
		if (e != NULL) {
			TAILQ_REMOVE(&env_sched_list, e, env_sched_link);
			if (e->env_status == ENV_RUNNABLE) {
				TAILQ_INSERT_TAIL(&env_sched_list, e, env_sched_link);
			}
		}

		if (TAILQ_EMPTY(&env_sched_list)) {
			panic("schedule: no runnable envs");
		}
```

随后，我们设定调度队列头部的进程控制块为将要运行的进程（不要在这里使用 `TAILQ_REMOVE`），将剩余时间片更新为新的进程的优先级。你可能会想：“所谓优先级就是时间片的多少？” 确实，就是这样，背后并没有什么复杂的算法。真正的操作系统中的进程优先级也仅仅是这样的作用。
```c
		e = TAILQ_FIRST(&env_sched_list);

		count = e->env_pri;
	}
```

最后，不管发不发生进程切换，我们都要让 `count` 自减，表示当前进程用去了一个时间片的时间，之后我们调用 `env_run` 运行进程，来真正的消耗这一时间片的时间。这个调用位于判断语句之外，也就是说不论是否发生进程切换，都需要执行 `env_run`。
```c
	count--;
	env_run(e);
}
```

### （3）进程的运行
`env_run` 是如何让进程运行的？让我们查看这个函数，它位于 kern/env.c 中。

唠叨一句无关的。在该函数的开头调用了 `pre_env_run` 用于打印评测信息。说实话有点丑。
```c
void env_run(struct Env *e) {
	assert(e->env_status == ENV_RUNNABLE);
	pre_env_run(e); // WARNING: DO NOT MODIFY THIS LINE!
```

此时全局变量 `curenv` 中还是切换前的进程控制块，我们保存该进程的上下文，将栈帧中 trap frame 的信息转换为 `Trapframe` 存储在 `env_tf` 中。至于为什么 trap frame 的信息存储在 `[KSTACKTOP - 1, KSTACKTOP)` 的范围内，参考关于 `SAVE_ALL` 宏的内容。
```c
	/* Step 1:
	 *   If 'curenv' is NULL, this is the first time through.
	 *   If not, we may be switching from a previous env, so save its context into
	 *   'curenv->env_tf' first.
	 */
	if (curenv) {
		curenv->env_tf = *((struct Trapframe *)KSTACKTOP - 1);
	}
```

接着，我们将 `curenv` 的值变为 `e` 的值，实现当前进程的切换。注释 `lab6` 的内容暂不考虑。
```c
	/* Step 2: Change 'curenv' to 'e'. */
	curenv = e;
	curenv->env_runs++; // lab6
```

之后，我们将全局变量 `cur_pgdir` 设置为当前进程对应的页目录，实现页目录的切换。各位或许还对 `cur_pgdir` 有印象，在 Lab2 的笔记中有提到。那时我说
> 值得注意的是，`_do_tlb_refill` 调用该函数时页目录基地址参数使用的是全局变量 `cur_pgdir`。可是这个全局变量并没有任何被赋值。这也是在 Lab2 中页式内存管理无法使用的一个原因。
现在我们为 `cur_pgdir` 赋了值，就可以愉快地使用用户内存空间范围的虚拟地址了。
```c
	/* Step 3: Change 'cur_pgdir' to 'curenv->env_pgdir', switching to its address space. */
	/* Exercise 3.8: Your code here. (1/2) */
	cur_pgdir = curenv->env_pgdir;
```

最后，我们调用 `env_pop_tf` 函数，根据栈帧还原进程上下文，并运行程序。
```c
	/* Step 4: Use 'env_pop_tf' to restore the curenv's saved context (registers) and return/go
	 * to user mode.
	 *
	 * Hint:
	 *  - You should use 'curenv->env_asid' here.
	 *  - 'env_pop_tf' is a 'noreturn' function: it restores PC from 'cp0_epc' thus not
	 *    returning to the kernel caller, making 'env_run' a 'noreturn' function as well.
	 */
	/* Exercise 3.8: Your code here. (2/2) */
	env_pop_tf(&curenv->env_tf, curenv->env_asid);
}
```

`env_pop_tf` 函数定义在 kern/env_asm.S 中，内容较为简单。该函数将传入的 asid 值设置到 `EntryHi` 寄存器中，表示之后的虚拟内存访问都来自于 asid 所对应的进程。另外该函数将`sp` 寄存器地址设置为当前进程的 trap frame 地址，这样在最后调用 `ret_from_exception` 从异常处理中返回时，将使用当前进程的 trap frame 恢复上下文。程序也将从当前进程的 epc 中执行（epc 的值在 `load_icode` 中根据 elf 头设置为程序入口地址）。
```c
LEAF(env_pop_tf)
.set reorder
.set at
	sll     a1, a1, 6
	mtc0    a1, CP0_ENTRYHI
	move    sp, a0
	j       ret_from_exception
END(env_pop_tf)
```

最后，所有的寄存器都恢复成了当前进程所需要的状态，cpu 就像只知道当前进程这一个程序一样不断执行一条条指令，直到经过了一个时钟周期，又一个中断发生……

**（Lab3 完）**
