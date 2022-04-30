// Do not forget add #undef for new #define's
#ifdef __cplusplus
extern "C" {
#define BOOT_STD_RESTRICT
#define BOOT_STD_NORETURN [[noreturn]]
#define BOOT_STD_NOEXCEPT noexcept
#else
#define BOOT_STD_RESTRICT restrict
#define BOOT_STD_NORETURN _Noreturn
#define BOOT_STD_NOEXCEPT
#endif
