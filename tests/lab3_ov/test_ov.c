int main() {
	unsigned int src1, src2, dst;

	/*
	 * 测试 add
	 */
	src1 = 0x80000000;
	src2 = 0x80000000;
	asm volatile("add %0,%1,%2\n\t"
		     : "=r"(dst) /* 输出操作数，也是第0个操作数%0 */
		     : "r"(src1), "r"(src2) /* 输入操作数，也是第1个操作数和第2个操作数 %1,%2 */
	);
	/* 这一句代码相当于运行了一条add指令，其中源操作数来自src1和src2，目的操作数将储存到dst中。*/
	if (dst != (src1 + src2)) {
		return -1; // 如果异常处理结果不正确进程将运行结束并返回-1
	}

	/*
	 * 测试 sub
	 */
	src1 = 0x80000000;
	src2 = 0x70000000;
	asm volatile("sub %0,%1,%2\n\t" : "=r"(dst) : "r"(src1), "r"(src2));
	if (dst != (src1 - src2)) {
		return -1;
	}

	/*
	 * 测试 addi
	 */
	src1 = 0x7ffffff0;
	asm volatile("addi %0, %1, 20\n\t" : "=r"(dst) : "r"(src1));
	if (dst != (src1 / 2 + 10)) {
		return -1;
	}
	return dst;
}
