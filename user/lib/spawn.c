#include <elf.h>
#include <env.h>
#include <lib.h>
#include <mmu.h>

#define debug 0

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

	// Make sure everything will fit in the initial stack page
	if (ROUND(tot, 4) + 4 * (argc + 3) > BY2PG) {
		return -E_NO_MEM;
	}

	// Determine where to place the strings and the args array
	strings = (char *)(UTEMP + BY2PG) - tot;
	args = (u_int *)(UTEMP + BY2PG - ROUND(tot, 4) - 4 * (argc + 1));

	if ((r = syscall_mem_alloc(0, (void *)UTEMP, PTE_D)) < 0) {
		return r;
	}

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

	// Initialize args[0..argc-1] to be pointers to these strings
	// that will be valid addresses for the child environment
	// (for whom this page will be at USTACKTOP-BY2PG!).
	ctemp = (char *)(USTACKTOP - UTEMP - BY2PG + (u_int)strings);
	for (i = 0; i < argc; i++) {
		args[i] = (u_int)ctemp;
		ctemp += strlen(argv[i]) + 1;
	}

	// Set args[argc] to 0 to null-terminate the args array.
	ctemp--;
	args[argc] = (u_int)ctemp;

	// Push two more words onto the child's stack below 'args',
	// containing the argc and argv parameters to be passed
	// to the child's main() function.
	u_int *pargv_ptr;
	pargv_ptr = args - 1;
	*pargv_ptr = USTACKTOP - UTEMP - BY2PG + (u_int)args;
	pargv_ptr--;
	*pargv_ptr = argc;

	// Set *init_sp to the initial stack pointer for the child
	*init_sp = USTACKTOP - UTEMP - BY2PG + (u_int)pargv_ptr;

	if ((r = syscall_mem_map(0, (void *)UTEMP, child, (void *)(USTACKTOP - BY2PG), PTE_D)) <
	    0) {
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

	const Elf32_Ehdr *ehdr = elf_from(elfbuf, sizeof(Elf32_Ehdr));
	if (!ehdr) {
		r = -E_NOT_EXEC;
		goto err;
	}
	u_long entrypoint = ehdr->e_entry;

	// Step 3: Create a child using 'syscall_exofork()' and store its envid in 'child'.
	// If the syscall fails, set 'r' and 'goto err'.
	u_int child;
	/* Exercise 6.4: Your code here. (2/6) */

	// Step 4: Use 'init_stack(child, argv, &sp)' to initialize the stack of the child.
	// 'goto err1' if that fails.
	u_int sp;
	/* Exercise 6.4: Your code here. (3/6) */

	// Step 5: Load the ELF segments in the file into the child's memory.
	// This is similar to 'load_icode()' in the kernel.
	size_t ph_off;
	ELF_FOREACH_PHDR_OFF (ph_off, ehdr) {
		// Read the program header in the file with offset 'ph_off' and length
		// 'ehdr->e_phentsize' into 'elfbuf'.
		// 'goto err1' on failure.
		// You may want to use 'seek' and 'readn'.
		/* Exercise 6.4: Your code here. (4/6) */

		Elf32_Phdr *ph = (Elf32_Phdr *)elfbuf;
		if (ph->p_type == PT_LOAD) {
			void *bin;
			// Read and map the ELF data in the file at 'ph->p_offset' into our memory
			// using 'read_map()'.
			// 'goto err1' if that fails.
			/* Exercise 6.4: Your code here. (5/6) */

			// Load the segment 'ph' into the child's memory using 'elf_load_seg()'.
			// Use 'spawn_mapper' as the callback, and '&child' as its data.
			// 'goto err1' if that fails.
			/* Exercise 6.4: Your code here. (6/6) */

		}
	}
	close(fd);

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

	if ((r = syscall_set_env_status(child, ENV_RUNNABLE)) < 0) {
		debugf("spawn: syscall_set_env_status %x: %d\n", child, r);
		goto err2;
	}
	return child;

err2:
	syscall_env_destroy(child);
	return r;
err1:
	syscall_env_destroy(child);
err:
	close(fd);
	return r;
}

int spawnl(char *prog, char *args, ...) {
	// Thanks to MIPS calling convention, the layout of arguments on the stack
	// are straightforward.
	return spawn(prog, &args);
}
