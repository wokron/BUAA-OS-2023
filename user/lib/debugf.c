#include <lib.h>
#include <print.h>

#define BUF_LEN 1024

struct debug_ctx {
	char buf[BUF_LEN];
	size_t pos;
};

static void debug_flush(struct debug_ctx *ctx) {
	if (ctx->pos == 0) {
		return;
	}
	int r;
	if ((r = syscall_print_cons(ctx->buf, ctx->pos)) != 0) {
		user_panic("syscall_print_cons: %d", r);
	}
	ctx->pos = 0;
}

static void debug_output(void *data, const char *s, size_t l) {
	struct debug_ctx *ctx = (struct debug_ctx *)data;

	while (ctx->pos + l > BUF_LEN) {
		size_t n = BUF_LEN - ctx->pos;
		memcpy(ctx->buf + ctx->pos, s, n);
		s += n;
		l -= n;
		ctx->pos = BUF_LEN;
		debug_flush(ctx);
	}
	memcpy(ctx->buf + ctx->pos, s, l);
	ctx->pos += l;
}

static void vdebugf(const char *fmt, va_list ap) {
	struct debug_ctx ctx;
	ctx.pos = 0;
	vprintfmt(debug_output, &ctx, fmt, ap);
	debug_flush(&ctx);
}

void debugf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vdebugf(fmt, ap);
	va_end(ap);
}

void _user_panic(const char *file, int line, const char *fmt, ...) {
	debugf("panic at %s:%d: ", file, line);
	va_list ap;
	va_start(ap, fmt);
	vdebugf(fmt, ap);
	va_end(ap);
	debugf("\n");
	exit();
}

void _user_halt(const char *file, int line, const char *fmt, ...) {
	debugf("halt at %s:%d: ", file, line);
	va_list ap;
	va_start(ap, fmt);
	vdebugf(fmt, ap);
	va_end(ap);
	debugf("\n");
	syscall_panic("user halt");
}
