#include <lib.h>
#include <print.h>

struct print_ctx {
	int fd;
	int ret;
};

static void print_output(void *data, const char *s, size_t l) {
	struct print_ctx *ctx = (struct print_ctx *)data;
	if (ctx->ret < 0) {
		return;
	}
	int r = write(ctx->fd, s, l);
	if (r < 0) {
		ctx->ret = r;
	} else {
		ctx->ret += r;
	}
}

static int vfprintf(int fd, const char *fmt, va_list ap) {
	struct print_ctx ctx;
	ctx.fd = fd;
	ctx.ret = 0;
	vprintfmt(print_output, &ctx, fmt, ap);
	return ctx.ret;
}

int fprintf(int fd, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int r = vfprintf(fd, fmt, ap);
	va_end(ap);
	return r;
}

int printf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int r = vfprintf(1, fmt, ap);
	va_end(ap);
	return r;
}
