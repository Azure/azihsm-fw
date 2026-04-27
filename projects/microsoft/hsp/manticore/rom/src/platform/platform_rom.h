#ifndef PLATFORM_ROM_H_
#define PLATFORM_ROM_H_


/* ROM does not not have a heap or support malloc/free calls. */
#undef	platform_malloc
#define	platform_malloc(x)		NULL

#undef	platform_calloc
#define	platform_calloc(x)		NULL

#undef	platform_realloc
#define	platform_realloc(x)		NULL

#undef	platform_free
#define	platform_free(x)		(void) x


#endif /* PLATFORM_ROM_H_ */
