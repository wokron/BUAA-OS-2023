## 一、Lab2 前言
这篇文章应该是我目前写过的文章中长度排行前几的了。Lab2 的内容着实繁多，不仅是分页内存管理本身的理论和实现细节颇多；操作系统的基本知识和注意事项也占据了很大的篇幅。后者在不理解的情况下实在会对本次实验产生许多困惑。本人也是在逐步地探索之后才得以有了较多的认识——当然，这一认识或许也只是片面的。

本文逐函数、逐代码地讲解了 Lab2 中新增的内容。主要在于内核初始化中关于内存的部分以及分页内存管理的实现。在本文中，关于链表宏和虚拟/物理内存的辨析也占据了比较多的内容。

<!-- more -->

## 二、内核初始化（续）
在 Lab1 中，我们的内核初始化过程只进行了一部分。因为 Lab1 中 `mips_init` 函数几乎没有任何功能。在 Lab2 中，我们会继续推进这一过程。

在 Lab2 中，我们会建立操作系统的内存管理机制。具体来说，我们会在 `mips_init` 中调用三个函数 `mips_detect_memory`、`mips_vm_init` 和 `page_init`。这三个函数会分别完成探测内存、初始化虚拟地址和初始化页的工作。接下来我们会分别介绍这三个函数。

Lab2 中 `mips_init` 的结构如下：
```c
void mips_init() {
	printk("init.c:\tmips_init() is called\n");

	// lab2:
	mips_detect_memory();
	mips_vm_init();
	page_init();

	while (1) {
	}
}
```

### （1）探测内存
`mips_detect_memory` 的作用是获取总物理内存大小，并根据物理内存计算分页数。
> 注意！是物理内存

```c
void mips_detect_memory() {
	/* Step 1: Initialize memsize. */
	memsize = *(volatile u_int *)(KSEG1 | DEV_MP_ADDRESS | DEV_MP_MEMORY)
```
第一步中的这条语句似乎使人困惑。为什么这样就可以获得物理内存大小了呢？我们可以查看一下 `DEV_MP_ADDRESS` 和 `DEV_MP_MEMORY` 所在的头文件。它们定义在 include/driver/dev_mp.h 中。

恰好 include/driver 目录下有一个 README，其中提到
```text
The files in this directory describe the devices found in GXemul's "test
machines". These machines do not match any real-world machines, but they
have devices that are similar to those found in real machines:

omit...

  o)  mp (dev_mp):
	A multiprocessor inter-processor communication device.
	It also contains other useful functionality, such as retrieving
	the amount of "physical" RAM installed in the emulated machine.

omit...
```
这就说明 dev_mp.h 中的信息应该是关于 multiprocessor inter-processor communication device (mp) 的相关信息的。这是 GXemul 定义的虚拟设备。mp 能够检索（retrive）RAM 数量（？amount）。这就足够了。

再看 dev_mp.h。通过注释我们可以得知，这个头文件中定义的 `DEV_MP_ADDRESS` 是（mp 设备）默认的物理基地址。而 `DEV_MP_MEMORY` 是物理基地址到 “设备寄存器” 的偏移量。
```c
/*
 *  Default (physical) base address and length:
 */

#define DEV_MP_ADDRESS 0x11000000ULL
#define DEV_MP_LENGTH 0x00000100ULL

// omit...

/*
 *  Offsets from the base address to reach the MP device' registers:
 */

#define DEV_MP_MEMORY 0x0090

// omit...
```

现在我们就可以解释 `DEV_MP_ADDRESS | DEV_MP_MEMORY` 的含义了。这表示一个物理地址，该地址正好对应 mp 设备的 `MEMORY` 寄存器。其中存储了物理内存大小的相关的信息。

但是在代码中，我们只能使用虚拟地址进行访存。因此我们需要得到该物理地址对应的虚拟地址。这时候 `KSEG1` 就派上用场了。根据指导书我们知道，kseg1 段的虚拟地址转换为物理地址只需要将最高 3 位置 0，不通过 TLB，同时也不通过 cache。实际上，kseg1 段就是为访问外设准备的。

我们将物理地址 `DEV_MP_ADDRESS | DEV_MP_MEMORY` 转换为虚拟地址 `KSEG1 |`，指明该地址指向的是一个无符号整数 `(volatile u_int *)`，最后取出该位置的值 `memsize = *`。这就是这一条语句的含义。

话说回来，在 Lab1 的 `printcharc` 函数中我们也遇到过类似的写法：`KSEG1 + DEV_CONS_ADDRESS + DEV_CONS_PUTGETCHAR`。但是当时没有进一步说明。本使用中的使用的是按位或而非加法，也进一步加深了代码的迷惑程度。

> 我在这里需要插一句，因为我不知道要把这部分内容放到哪里。
> 
> 根据指导书可以知道，kseg1 段位于 `0xa0000000~0xbfffffff`，映射的物理地址为 `0x00000000~0x1fffffff`。同样 kseg0 段位于 `0x80000000~0x9fffffff`，但映射的物理地址也为 `0x00000000~0x1fffffff`。这是否出错了呢？其实不是。
>
> kseg0 和 kseg1 中两个不同的虚拟地址，其实就对应同一个物理地址。区分 kseg0 和 kseg1 的目的，就在于区分是否使用 cache。
>
> 同样的，其实 kuseg 映射的物理地址也和 kseg0 和 kseg1 相同。你可能会想：“不对呀，kuseg 的虚拟地址空间明显大于 kseg0 和 kseg1。怎么可能映射到同样的物理地址空间？” 其实解决问题的关键就在于页表。这在本篇文章的后面会详细讲解。

终于分析完第一条语句了。`mips_detect_memory` 中剩下的部分就比较简单了。
```c
	/* Step 2: Calculate the corresponding 'npage' value. */
	/* Exercise 2.1: Your code here. */
	npage = memsize / BY2PG;

	printk("Memory size: %lu KiB, number of pages: %lu\n", memsize / 1024, npage);
}
```
在这部分中，我们将总物理内存大小除以页大小，得到总页数。并调用 Lab1 中编写的 `printk` 输出相关信息。需要注意的是 `BY2PG` 是一个宏，定义在 include/mmu.h 中。根据注释可以得知这表示一页的字节数。因此除以 `BY2PG` 即可得到总页数。
```c
#define BY2PG 4096		// bytes to a page
```

### （2）初始化虚拟地址
接着我们考虑 `mips_vm_init` 函数。这个函数将申请一部分空间用作页控制块。页控制块是 `struct Page` 类型的结构体。每一个页控制块对应一个物理页。

`struct Page` 的结构很简单，只不过因为使用了链表宏（链表宏会在之后讲解），导致不容易理解。如下是展开后的 `struct Page`
```c
struct Page {
	struct {
		struct Page *le_next;
		struct Page **le_prev;
	} pp_link;
	u_short pp_ref;
};
```

其中只有一个用于表示链表前后节点的结构体 `pp_link`；以及用于引用计数，反映页的使用情况的 `pp_ref`。

这么简单的结构是如何映射到物理页的呢？其实也很简单。在 include/pmap.h 中我们可以得知，所有的页控制块都保存在一个数组中 `extern struct Page *pages`（这也是我们将要在`mips_vm_init`申请的数组）。

通过指针减法，可以得到对应的页控制块是第几个页
```c
static inline u_long page2ppn(struct Page *pp) {
	return pp - pages;
}
```

一个物理页有确定的大小，我们将第几个页乘以物理页大小即可得到对应物理页的基地址。`page2pa` 表示 “page to physical addresss” 的意思。
```c
// in mmu.h
#define PGSHIFT 12

// in pmap.h
static inline u_long page2pa(struct Page *pp) {
	return page2ppn(pp) << PGSHIFT;
}
```

同样的，还有反过程。通过物理地址获取对应的页控制块。`PPN` 宏获取物理地址对应的页数，`pa2page` 根据该页数求对应的页控制块。
```c
// in mmu.h
#define PPN(va) (((u_long)(va)) >> 12)

// in pmap.h
static inline struct Page *pa2page(u_long pa) {
	if (PPN(pa) >= npage) {
		panic("pa2page called with invalid pa: %x", pa);
	}
	return &pages[PPN(pa)];
}
```

这样页控制块就大概讲清楚了。让我们回到 `mips_vm_init`。其中真正重要的只有第一句。在这一句中，我们调用了 `alloc` 函数申请了 `npage` 个 `struct Page` 大小的内存。并以 `BY2PG`（页的大小）进行对齐。同时将申请的内存中内容初始化为 0。这到底是什么意思？还是让我们看一下 `alloc` 的定义。
```c
void mips_vm_init() {
	/* Allocate proper size of physical memory for global array `pages`,
	 * for physical memory management. Then, map virtual address `UPAGES` to
	 * physical address `pages` allocated before. For consideration of alignment,
	 * you should round up the memory size before map. */
	pages = (struct Page *)alloc(npage * sizeof(struct Page), BY2PG, 1);
	printk("to memory %x for struct Pages.\n", freemem);
	printk("pmap.c:\t mips vm init success\n");
}
```

在内核启动阶段，我们还没有什么像样的内存申请方式。因此只能自己写一个。在 `alloc` 函数中，我们首先定义了两个变量。
```c
void *alloc(u_int n, u_int align, int clear) {
	extern char end[];
	u_long alloced_mem;
```

`alloced_mem` 是在程序中表示已分配内存的变量。`end` 是一个外部定义的变量，我们可以在 kernel.lds 中找到其对应的值。这在 Lab1 中也有所提及。需要注意的是，这里使用的是虚拟地址。查看内存分布表可知，此虚拟地址位于 kseg0 中。我们将内核的代码与数据结构都存储到 kseg0。接下来的内容都是在 kseg0 中进行。
```c
	. = 0x80400000;
	end = . ;
```

回到 `alloc`。第一步，我们先对 `freemem` 的值进行初始化。`freemem` 是用来表示可用内存地址的全局变量。
```c
	if (freemem == 0) {
		freemem = (u_long)end; // end
	}
```

接着，我们把 `freemem` 以参数 `align` 对齐。这样接下来我们分配的内存才能从能被参数 `align` 整除的地址开始。
```c
	/* Step 1: Round up `freemem` up to be aligned properly */
	freemem = ROUND(freemem, align);
```

`ROUND` 宏定义在 include/types.h 中。只能对齐 2 的整数幂。主要原理是将低位抹零。`ROUND` 宏还有一个对应的宏 `ROUNDDOWN`。前者向上对齐，后者向下对齐。
```c
/* Rounding; only works for n = power of two */
#define ROUND(a, n) (((((u_long)(a)) + (n)-1)) & ~((n)-1))
#define ROUNDDOWN(a, n) (((u_long)(a)) & ~((n)-1))
```

在 `align` 中，我们现在确定已分配空间的上界，继续分配参数 `n` 个字节的内存。
```c
	/* Step 2: Save current value of `freemem` as allocated chunk. */
	alloced_mem = freemem;

	/* Step 3: Increase `freemem` to record allocation. */
	freemem = freemem + n;
```

如果需要清零，则使用 `memset` 函数清零。接着返回 `alloced_mem` 的地址。
```c
/* Step 4: Clear allocated chunk if parameter `clear` is set. */
	if (clear) {
		memset((void *)alloced_mem, 0, n);
	}

	/* Step 5: return allocated chunk. */
	return (void *)alloced_mem;
}
```

值得注意的是中间还有一个类似于 `assert` 的语句
```c
	// Panic if we're out of memory.
	panic_on(PADDR(freemem) >= memsize);
```

需要说明的是其中用到的的 `PADDR` 宏。这个宏将 kseg0 中的虚拟地址转化为物理地址。`ULIM` 是 kseg0 的基地址，因此 `a - ULIM` 等价于最高三位抹零。`PADDR` 还有一个对应宏 `KADDR`，将物理地址转换为 kseg0 中的内核虚拟地址，只不过是将减号改为加号。
```c
#define PADDR(kva) \
	({ \
		u_long a = (u_long)(kva); \
		if (a < ULIM) \
			panic("PADDR called with invalid kva %08lx", a); \
		a - ULIM; \
	})
```

`memsize` 是物理内存的大小，当物理地址大于 `memsize` 时也就说明其超出了内存。

好了，现在 `alloc` 也讲解完了。你可能会想：“这分配了什么，不是就直接返回了个指针么？” 确实。说白了内存你本就可以随意使用，申请内存不过是为了避免内存使用冲突的机制罢了。

当讲完了 `alloc` 后，`mips_vm_init` 的内容也就明了了。我们创建了一个 `struct Page` 的数组，大小为 `npage`。

### （3）初始化页
在 `page_init` 中，我们将对 `mips_vm_init` 中申请的数组内容进行初始化，并维护一个存储所有空闲页的链表。

在正式开始之前，需要介绍一下在 include/queue.h 中定义的双向链表宏。通过使用宏，我们在 c 语言中实现了泛型。

为了使用链表，我们需要定义两个结构 `LIST_HEAD` 和 `LIST_ENTRY`。前者表示链表头或链表本身的类型，后者表示链表中元素的类型。通过宏定义可知，`LIST_HEAD(name, type)` 表示创建一个元素类型为 `type` 的链表，这个链表类型名为 `name`。`LIST_ENTRY(type)` 表示创建一个类型为 `type` 的链表元素。
```c
#define LIST_HEAD(name, type) \
	struct name { \
		struct type *lh_first; /* first element */ \
	}

#define LIST_ENTRY(type) \
	struct { \
		struct type *le_next;  /* next element */ \
		struct type **le_prev; /* address of previous next element */ \
	}
```

我们在 include/pmap.h 中就定义了元素为 `Page`，类型名为 `Page_list` 的链表。可以注意到 `struct Page` 的原始定义中包含了链表元素类型 `Page_LIST_entry_t`
```c
LIST_HEAD(Page_list, Page);
typedef LIST_ENTRY(Page) Page_LIST_entry_t;

struct Page {
	Page_LIST_entry_t pp_link; /* free list link */

	u_short pp_ref;
};

extern struct Page_list page_free_list;
```

include/queue.h 中也定义了一些链表操作，因为原理相似，在这里只介绍 `LIST_INSERT_AFTER(listelm, elm, field)`。这个函数也是我们需要填写的。
```c
#define LIST_INSERT_AFTER(listelm, elm, field)                                                     \
	/* Exercise 2.2: Your code here. */  \
	do { \
		if ((LIST_NEXT((elm), field) = LIST_NEXT((listelm), field)) != NULL) \
			LIST_NEXT((listelm), field)->field.le_prev = &LIST_NEXT((elm), field); \
		LIST_NEXT((listelm), field) = (elm); \
		(elm)->field.le_prev = &LIST_NEXT((listelm), field); \
	} while (0)
```

对于代码的第一行，我们是容易理解的。在这两行中，我们先让 `elm` 的下一个元素指向 `listelm` 的下一个元素。若下一个元素不是 `NULL`，则还需要将这下一个元素的前一个元素设置为 `elm`。
```c
if ((LIST_NEXT((elm), field) = LIST_NEXT((listelm), field)) != NULL) \
			LIST_NEXT((listelm), field)->field.le_prev = &LIST_NEXT((elm), field); \
```

但这里出现了问题，为什么第二行使用的是 `&LIST_NEXT((elm), field)` 而非 `&elm`？重新看一下 `LIST_ENTRY` 的定义，可以发现对 le_prev 的注释是 `/* address of previous next element */`。(前一个(下一个元素))的地址，也就是说本来 `le_prev` 的地址就是上一个元素的 `le_next` 的地址。这样做有什么意义呢？叶gg说这样方便定义头指针。因为 `LIST_HEAD` 和 `LIST_ENTRY` 不是同一个类型，如果 `le_prev` 的类型是 `struct type *`，那么头结点也必须是 `type` 类型，这会浪费一个指针大小的空间。

> 有些文章可能会认为这个链表无法直接访问前节点，其实这应该是错的（因为如果是单向链表，那么根本没必要设计 `le_prev`）。

这样的话，代码的第三四行也可以理解了。`listelm` 的下一个元素是 `elm`。`elm` 的 `le_prev` 的值是前一个元素，`listelm` 的 `le_next` 的地址。
```c
LIST_NEXT((listelm), field) = (elm); \
(elm)->field.le_prev = &LIST_NEXT((listelm), field); \
```

现在我们理解了链表宏的含义，可以回来看 `page_init` 函数了。我们想一下接下来要做什么。我们有物理内存，并将其划分成了许多的页，这些页的信息通过页控制块保存在 `pages` 数组中。可是现在页控制块还没有被设置，具体来说，我们还没有明确哪些页是可用的，哪些页是已经被使用的。因此接下来我们要做到就是将页划分成可用和不可用的，并将可用的页控制块放入 `page_free_list` 中（这样想要申请新的页，只需要取出该链表的头结点即可）。

第一步，我们初始化链表（实际上只是将头结点的指针值设为 `NULL`）。
```c
void page_init(void) {
	/* Step 1: Initialize page_free_list. */
	/* Hint: Use macro `LIST_INIT` defined in include/queue.h. */
	/* Exercise 2.3: Your code here. (1/4) */
	LIST_INIT(&page_free_list);
```

然后我们确定已使用内存的最大地址，为了适配页的大小，需要进行对齐
```c
	/* Step 2: Align `freemem` up to multiple of BY2PG. */
	/* Exercise 2.3: Your code here. (2/4) */
	freemem = ROUND(freemem, BY2PG);
```

接着，我们需要将已使用的页的引用数设为 1，表示页已经被使用。首先我们计算有多少已使用的页，我们先使用 `PADDR` 将 `freemem` 转换为物理地址，接着使用 `PPN` 获取该地址属于第几个页表。使用一个循环将前 `usedpage` 个页控制块的 `pp_ref` 设置为 1。 
```c
/* Step 3: Mark all memory below `freemem` as used (set `pp_ref` to 1) */
/* Exercise 2.3: Your code here. (3/4) */
u_long usedpage = PPN(PADDR(freemem));

for (u_long i = 0; i < usedpage; i++) {
	pages[i].pp_ref = 1;
}
```

最后，我们将剩下的页控制块的 `pp_ref` 设置为 0，并将这些页控制块插入到 `page_free_list` 中。
```c
	/* Step 4: Mark the other memory as free. */
	/* Exercise 2.3: Your code here. (4/4) */
	for (u_long i = usedpage; i < npage; i++) {
		pages[i].pp_ref = 0;
		LIST_INSERT_HEAD(&page_free_list, &pages[i], pp_link);
	}
}
```

## 三、页式内存管理
当我们使用 kuseg 地址空间的虚拟地址访问内存时，我们会通过 TLB 将其转换为物理地址。当 TLB 中查询不到对应的物理地址时，就会发生 TLB Miss 异常。这时将跳转到异常处理函数，执行 TLB 重填。在 Lab2，我们的代码还未启用异常处理，因此无法真正运行页式内存管理机制，但是代码中已经定义了 TLB 重填函数。我们将从此开始解读 MOS 中的页式内存管理。

> 注意，页式内存管理部分各类函数杂糅在一起。水平有限，以调用过程叙述时实在难以保证行文结构，因此小节题目不一定完全概括小节内容

### （1）TLB 重填
TLB 的重填过程由 kern/tlb_asm.S 中的 `do_tlb_refill` 函数完成。该函数是汇编实现的。首先，定义了一个字的变量，标签为 `tlb_refill_ra`
```c
.data
tlb_refill_ra:
.word 0
```

接着是代码部分。首先我们使用 `NESTED` 定义函数标签。`NESTED` 与 `LEAF` 宏相对应。前者表示非叶函数，后者表示叶函数。
```c
.text
NESTED(do_tlb_refill, 0, zero)
```

我们希望汇编尽可能少，因此希望 `do_tlb_refill` 只做必要的处理，随后调用 c 函数进一步处理。因此首先我们设置参数。第一个参数是 `BadVAddr` 寄存器的值，即发生 TLB Miss 的虚拟地址；第二个参数是 `EntryHi` 寄存器的 6-11 位。即当前进程的 ASID。
```c
	mfc0    a0, CP0_BADVADDR
	mfc0    a1, CP0_ENTRYHI
	srl     a1, a1, 6
	andi    a1, a1, 0b111111
```

接着我们调用 c 函数 `_do_tlb_refill`（这个函数会在后面说明）。注意这里存储了原来的 `ra` 寄存器值。
```c
	sw      ra, tlb_refill_ra
	jal     _do_tlb_refill
	lw      ra, tlb_refill_ra
```

`_do_tlb_refill` 会返回虚拟地址对应的页表项。我们将该返回值存入 `EntryLo`，并将 `EntryHi` 和 `EntryLo` 的值写入 TLB。
```c
	mtc0    v0, CP0_ENTRYLO0
	// See <IDT R30xx Family Software Reference Manual> Chapter 6-8
	nop
	/* Hint: use 'tlbwr' to write CP0.EntryHi/Lo into a random tlb entry. */
	/* Exercise 2.10: Your code here. */
	tlbwr

	jr      ra
END(do_tlb_refill)
```

这样就完成了 TLB 重填。跳回到正常程序后，此前产生异常的虚拟地址就可以通过 TLB 访问内存了。

接着我们详细深入 `_do_tlb_refill`，这个函数在 kern/tlbex.c 中。正如 hints 所说，在这个函数中，我们会不断查找虚拟地址对应的页表项，如果未找到，则试图申请一个新的页表项。最终返回申请到的页表项的内容。
```c
Pte _do_tlb_refill(u_long va, u_int asid) {
	Pte *pte;
	/* Hints:
	 *  Invoke 'page_lookup' repeatedly in a loop to find the page table entry 'pte' associated
	 *  with the virtual address 'va' in the current address space 'cur_pgdir'.
	 *
	 *  **While** 'page_lookup' returns 'NULL', indicating that the 'pte' could not be found,
	 *  allocate a new page using 'passive_alloc' until 'page_lookup' succeeds.
	 */

	/* Exercise 2.9: Your code here. */
	while (page_lookup(cur_pgdir, va, &pte) == NULL) {
		passive_alloc(va, cur_pgdir, asid);
	}

	return *pte;
}
```

### （2）页的查找
接着我们详细讨论 `_do_tlb_refill` 中所使用的函数。

`page_lookup` 函数在 kern/pmap.c 中定义。这个函数用于查找虚拟地址对应的页控制块及页表项。函数的参数是页目录的（虚拟）基地址，想要转换的虚拟地址和用于返回对应页表项的指针。值得注意的是，`_do_tlb_refill` 调用该函数时页目录基地址参数使用的是全局变量 `cur_pgdir`。可是这个全局变量并没有任何被赋值。这也是在 Lab2 中页式内存管理无法使用的一个原因。

我们继续看 `page_lookup` 的内容。其中首先调用了另一个函数 `pgdir_walk`。这个函数会获取想要转换的虚拟地址对应的（二级）页表项地址，通过 `pte` 返回。其中第三个参数 `create` 表示若未找到对应页表是否创建新的页表，此处为 0 表示不创建。
```c
struct Page *page_lookup(Pde *pgdir, u_long va, Pte **ppte) {
	struct Page *pp;
	Pte *pte;

	/* Step 1: Get the page table entry. */
	pgdir_walk(pgdir, va, 0, &pte);
```

接着 `page_lookup` 检查是否获取到对应的页表项，未获取到返回 `NULL`
```c
	/* Hint: Check if the page table entry doesn't exist or is not valid. */
	if (pte == NULL || (*pte & PTE_V) == 0) {
		return NULL;
	}
```

如果获取到，我们找到页表项对应的页控制块，并返回。
```c
	/* Step 2: Get the corresponding Page struct. */
	/* Hint: Use function `pa2page`, defined in include/pmap.h . */
	pp = pa2page(*pte);
	if (ppte) {
		*ppte = pte;
	}

	return pp;
}
```

> 需要注意这里有一个容易引起困惑的地方，`pte` 是虚拟地址对应的页表项的地址，`*pte` 是页表项的内容。我们知道页表项中除了物理地址之外还存储有其他信息。怎么就把其当做物理地址传入 `pa2page` 函数了呢？
>
> 让我们回到 `pa2page` 就知道了。在 `pa2page` 中我们通过 `PPN` 获取物理地址对应的第几页，而 `PPN` 是通过右移 12 位实现的。这样我们就将页表项中低位的用于表示权限等信息的内容消去，而只剩下页数了。（或许也正是因为 `*pte` 的低位无用，才将其用作其他内容。）

接着我们考察 `pgdir_walk` 函数，这个函数也在 kern/pmap.c 中定义。并且是需要我们填写的函数。如前所述，这个函数要实现查找对应虚拟地址对应的（二级）页表项，并根据 `create` 参数的设置在未找到二级页表时创建二级页表。

首先，我们根据虚拟地址确定对应的页目录项的地址。
```c
static int pgdir_walk(Pde *pgdir, u_long va, int create, Pte **ppte) {
	Pde *pgdir_entryp;
	struct Page *pp;

	/* Step 1: Get the corresponding page directory entry. */
	/* Exercise 2.6: Your code here. (1/3) */
	pgdir_entryp = pgdir + PDX(va);
```

其中使用了 `PDX` 宏。这个宏定义在 include/mmu.h 中。用于获取虚拟地址的 22-31 位的数值，这是虚拟地址对应的页目录项相对于页目录基地址的偏移。
```c
#define PDX(va) ((((u_long)(va)) >> 22) & 0x03FF)
```

随后我们判断该页目录项是否有效。如果无效，判断是否需要创建新的二级页表。如需要则使用 `page_alloc` 函数申请一个物理页，并设置虚拟地址对应页目录项的内容 `*pgdir_entryp = page2pa(pp) | PTE_D | PTE_V`，使其与该物理页关联。
```c
	if (!(*pgdir_entryp & PTE_V)) {
			if (create) {
				if (page_alloc(&pp) != 0) {
					return -E_NO_MEM;
				}
				pp->pp_ref++;
				*pgdir_entryp = page2pa(pp) | PTE_D | PTE_V;
```

`page_alloc` 函数是一个简单的函数，用于从 `page_free_list` 中抽取第一个空闲的页控制块，将页控制块对应的物理内存作为分配的内存。将该内存初始化为 0。唯一需要注意的是 `page2kva`。此函数实际上只是 `KADDR(page2pa(pp))`。
```c
int page_alloc(struct Page **new) {
	/* Step 1: Get a page from free memory. If fails, return the error code.*/
	struct Page *pp;
	/* Exercise 2.4: Your code here. (1/2) */
	if (LIST_EMPTY(&page_free_list)) {
		return -E_NO_MEM;
	}
	pp = LIST_FIRST(&page_free_list);

	LIST_REMOVE(pp, pp_link);

	/* Step 2: Initialize this page with zero.
	 * Hint: use `memset`. */
	/* Exercise 2.4: Your code here. (2/2) */
	memset((void *)page2kva(pp), 0, BY2PG);

	*new = pp;
	return 0;
}
```

回到 `pgdir_walk`，如果不需要创建，则直接返回
```c
		} else {
			*ppte = NULL;
			return 0;
		}
	}
```

函数中剩下的流程中，二级页表必然存在了。我们获取二级页表的虚拟基地址，并找到虚拟地址 `va` 对应的二级页表项，返回。
```c
	/* Step 3: Assign the kernel virtual address of the page table entry to '*ppte'. */
	/* Exercise 2.6: Your code here. (3/3) */
	Pte *pgtable = (Pte *)KADDR(PTE_ADDR(*pgdir_entryp));
	*ppte = pgtable + PTX(va);
	
	return 0;
}
```

需要注意这里我们使用了两个宏来获取二级页表基地址。第一个宏 `PTE_ADDR` 定义在 include/mmu.h 中。它返回页目录项对应的二级页表的基地址。实际上就是将页目录项内容的低 12 位抹零。如果是新申请的物理页作为二级页表，则该值实际上等于 `page2pa(pp)`。另一个宏 `KADDR` 将物理地址转换为 kseg0 的虚拟地址，不用细说。
```c
#define PTE_ADDR(pte) ((u_long)(pte) & ~0xFFF)
```

与 `PDX` 类似，`PTX` 宏返回虚拟地址 12-21 位的数值，`pgtable` 二级页表基地址加上偏移得到虚拟地址 `va` 对应的二级页表项。

### （3）页的申请
`pgdir_walk` 的内容我们已经分析完成，现在 `page_lookup` 函数也没有需要讲解的部分了。接下来我们分析 `_do_tlb_refill` 中的 `passive_alloc`。

`passive_alloc` 定义在 kern/tlbex.c 中。这是一个用于为虚拟地址申请物理页的函数。它的参数是：想要关联物理地址的虚拟地址、页目录的基地址和标识进程的 asid。
```c
static void passive_alloc(u_int va, Pde *pgdir, u_int asid) {
```

函数一开头就是好几条检查地址是否非法的判断语句，这里就不列出了。接下来的内容是，函数通过 `page_alloc` 申请一个物理页，并试图通过 `page_insert` 建立物理页和虚拟地址的联系。
```c
	panic_on(page_alloc(&p));
	panic_on(page_insert(pgdir, asid, p, PTE_ADDR(va), PTE_D));
}
```

`page_alloc` 已经在之前介绍过了，现在介绍 `page_insert`。这个函数定义在 kern/pmap.c 中。是我们需要补完的函数。

该函数首先调用 `pgdir_walk`，试图获取当前虚拟地址对应的二级页表项。
```c
int page_insert(Pde *pgdir, u_int asid, struct Page *pp, u_long va, u_int perm) {
	Pte *pte;
	
	/* Step 1: Get corresponding page table entry. */
	pgdir_walk(pgdir, va, 0, &pte);
```

如果确实获得了虚拟地址对应的二级页表项，并且是有效的，那么判断该页表项对应的物理页是否就是 `va` 想要映射的物理页（通过比较页控制块）。如果不一样，那么调用 `page_remove` 移除虚拟地址到原有的页的映射。`page_remove` 将在后续说明。
```c
	if (pte && (*pte & PTE_V)) {
		if (pa2page(*pte) != pp) {
			page_remove(pgdir, asid, va);
```

如果相同，说明虚拟地址已经映射到了对应的物理页。这时我们只需要更新一下页表项的权限 `*pte = page2pa(pp) | perm | PTE_V`。为了保证对页表的修改都能反映到 TLB 中，我们要调用 `tlb_invalidate` 函数将原有的关于 `va` 和 `asid` 的 TLB 表项清除。`tlb_invalidate` 将在后面说明。
```c
		} else {
			tlb_invalidate(asid, va);
			*pte = page2pa(pp) | perm | PTE_V;
			return 0;
		}
	}
```

程序执行 `page_insert` 的后续语句时，一定不存在虚拟地址 `va` 到页控制块对应的物理页的映射。于是接下来，我们就要建立这样的映射。首先我们还是要调用 `tlb_invalidate` 清除原有内容。
```c
	/* Step 2: Flush TLB with 'tlb_invalidate'. */
	/* Exercise 2.7: Your code here. (1/3) */
	tlb_invalidate(asid, va);
```

随后再调用一次 `pgdir_walk`，只不过这次 `create=1`。这将获得 `va` 对应的二级页表项
```c
	/* Step 3: Re-get or create the page table entry. */
	/* If failed to create, return the error. */
	/* Exercise 2.7: Your code here. (2/3) */
	if (pgdir_walk(pgdir, va, 1, &pte) != 0) {
		return -E_NO_MEM;
	}
```

最后，我们只需要建立二级页表项到物理页的联系即可。我们只需修改二级页表项的内容，修改为物理页的物理地址和权限设置即可。同时不要忘记递增页控制块的引用计数。
```c
	/* Step 4: Insert the page to the page table entry with 'perm | PTE_V' and increase its
	 * 'pp_ref'. */
	/* Exercise 2.7: Your code here. (3/3) */
	*pte = page2pa(pp) | perm | PTE_V;
	pp->pp_ref++;

	return 0;
}
```

### （4）页的移除
结束了 `page_insert` 的说明，让我们重新拾起按下不表的 `page_remove` 和 `tlb_invalidate`。我们首先考察 `page_remove`，此函数定义在 kern/pmap.c 中。用于取消虚拟地址 `va` 到物理页的映射。

首先该函数调用 `page_lookup` 查找与 `va` 和 `asid` 映射的物理页。如果不存在这样的页，则直接返回
```c
void page_remove(Pde *pgdir, u_int asid, u_long va) {
	Pte *pte;

	/* Step 1: Get the page table entry, and check if the page table entry is valid. */
	struct Page *pp = page_lookup(pgdir, va, &pte);
	if (pp == NULL) {
		return;
	}
```

如果存在，则调用 `page_decref` 以递减该页的引用数。当引用数等于零时，将该物理页重新放入未使用页的链表。因为对页表进行了修改，需要调用 `tlb_invalidate` 确保 TLB 中不保留原有内容。
```c
	/* Step 2: Decrease reference count on 'pp'. */
	page_decref(pp);

	/* Step 3: Flush TLB. */
	*pte = 0;
	tlb_invalidate(asid, va);
	return;
}
```

`page_decref` 定义如下，该函数和 `page_free` 都定义在 kern/pmap.c 中。这两个函数不需要讲解。
```c
void page_free(struct Page *pp) {
	assert(pp->pp_ref == 0);
	/* Just insert it into 'page_free_list'. */
	/* Exercise 2.5: Your code here. */
	LIST_INSERT_HEAD(&page_free_list, pp, pp_link);
}

void page_decref(struct Page *pp) {
	assert(pp->pp_ref > 0);

	/* If 'pp_ref' reaches to 0, free this page. */
	if (--pp->pp_ref == 0) {
		page_free(pp);
	}
}
```

还剩 `tlb_invalidate` 函数需要说明。这个函数主要用于调用另一个汇编函数 `tlb_out`。`tlb_invalidate` 将参数 `asid` 和 `va` 结合在了一起，传入 `tlb_out`。实际上这个结合在一起的参数就是 `EntryHi` 寄存器的结构。
```c
void tlb_invalidate(u_int asid, u_long va) {
	tlb_out(PTE_ADDR(va) | (asid << 6));
}
```

我们再考察 `tlb_out` 的内容。这部分定义在 kern/tlb_asm.S 中。首先可知，`tlb_out` 是一个叶函数。
```c
LEAF(tlb_out)
```

函数在一开始将原有的 `EnryHi` 寄存器中的值保存，并将传入的参数设置为 `EnryHi` 新的值。然后根据新的值查找 TLB 表项。
```c
set noreorder
	mfc0    t0, CP0_ENTRYHI
	mtc0    a0, CP0_ENTRYHI
	nop
	/* Step 1: Use 'tlbp' to probe TLB entry */
	/* Exercise 2.8: Your code here. (1/2) */
	tlbp // 这条指令根据 EntryHi 中的 Key，查找 TLB 中对应的表项，将该项的索引存入 Index 寄存器
	
	nop
```

随后将 `Index` 寄存器中的查询结果存储到 `t1` 寄存器，如果结果小于 0，说明未找到对应的表项，跳转到 NO_SUCH_ENTRY，不需要进行清零操作。
```c
	mfc0    t1, CP0_INDEX
.set reorder
	bltz    t1, NO_SUCH_ENTRY 
```

这里分别将 `EntryHi` 和 `EntryLo` 设置为 0。并将内容写入对应的表项，实现清零。
```c
.set noreorder
	mtc0    zero, CP0_ENTRYHI
	mtc0    zero, CP0_ENTRYLO0
	nop
	
	tlbwi
```

最后，恢复进入函数时 `EntryHi` 存储的值，函数返回。
```c
.set reorder

NO_SUCH_ENTRY:
	mtc0    t0, CP0_ENTRYHI

	j       ra
END(tlb_out)
```

这样就完成了 Lab2 中所有涉及到的代码的讲解。
