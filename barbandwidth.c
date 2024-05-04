//
// Util
//
#include <stdint.h>
#include <intrin.h>

typedef uint8_t     u8;
typedef int8_t      s8;
typedef uint32_t    u32;
typedef int32_t     s32;
typedef uint64_t    u64;
typedef int64_t     s64;
typedef uintptr_t   umm;
typedef intptr_t    smm;

typedef float       f32;
typedef double      f64;

typedef u32 flags32;
typedef u32 b32;

#define CountOf(a) (sizeof(a) / sizeof(*(a)))
#define Assert(expr) if (!expr) __debugbreak()

#define KiB(x) ((umm)(x) << 10)
#define MiB(x) ((umm)(x) << 20)

inline u64 Min(u64 A, u64 B) { return A < B ? A : B; }
inline u64 Max(u64 A, u64 B) { return A < B ? B : A; }

//
// Testing harness
//
typedef enum memory_type
{
    MemoryType_Host = 0,
    MemoryType_BAR,

    MemoryType_Count,
} memory_type;

typedef enum test_type
{
    TestType_Write = 0,
    TestType_Copy,
} test_type;

typedef void test_function(umm Count, void* Dst, void* Src);

typedef struct test_context
{
    u64 TSCFrequencyEstimate;
    umm BufferSize;
    void* Buffers[MemoryType_Count];
    char DeviceName[256];
} test_context;

typedef struct test_config
{
    const char*     Name;
    test_function*  Function;
    umm             Count;
    memory_type     MemoryType;
    test_type       TestType;
} test_config;

typedef struct test_result
{
    u64 Min;
    u64 Max;
    u64 Sum;
    umm DataProcessed;
} test_result;

static test_result RunTest(test_context* Context, test_config* Test)
{
    Assert(Context->BufferSize >= Test->Count);

    test_result Result = {0};
    Result.DataProcessed = Test->Count;
    Result.Min = ~(0llu);

    void* Dst = 0;
    void* Src = 0;
    switch (Test->TestType)
    {
        case TestType_Write:
        {
            Dst = Context->Buffers[Test->MemoryType];
        } break;
        case TestType_Copy:
        {
            Dst = Context->Buffers[MemoryType_BAR];
            Src = Context->Buffers[MemoryType_Host];
        } break;
    }

    const u32 RepCount = 4096;
    for (u32 Rep = 0; Rep < RepCount; Rep++)
    {
        u64 Begin = __rdtsc();
        Test->Function(Test->Count, Dst, Src);
        u64 End = __rdtsc();
        u64 Delta = End - Begin;

        Result.Min = Min(Result.Min, Delta);
        Result.Max = Max(Result.Max, Delta);
        Result.Sum += Delta;
    }

    f64 GhzConv = Context->TSCFrequencyEstimate / (1000.0 * 1000.0 * 1000.0);
    printf("=== %s\nMin:\t%f c/b (%f GB/s)\nMax:\t%f c/b (%f GB/s)\nSum:\t%f c/b (%f GB/s)\n",
           Test->Name, 
           Result.Min / (f64)Result.DataProcessed, GhzConv * (f64)Result.DataProcessed / (f64)Result.Min,
           Result.Max / (f64)Result.DataProcessed, GhzConv * (f64)Result.DataProcessed / (f64)Result.Max,
           Result.Sum / ((f64)Result.DataProcessed * RepCount), GhzConv * (f64)Result.DataProcessed * RepCount / (f64)Result.Sum);

    return(Result);
}

//
// App
//
#include <stdio.h>

void Write32x1              (umm Count, void* Dst, void* Src);
void Write32x2              (umm Count, void* Dst, void* Src);
void Write32x4              (umm Count, void* Dst, void* Src);
void WriteNonTemporal32x4   (umm Count, void* Dst, void* Src);
void Copy32x4               (umm Count, void* Dst, void* Src);
void CopyNonTemporal32x4    (umm Count, void* Dst, void* Src);

#define TestCopyTemporal        1
#define TestCopyNonTemporal     1
#define TestWriteNonTemporal    0
#define TestWriteTemporal       0

static test_config Tests[] = 
{
#if TestCopyNonTemporal
    { "Copy Non-Temporal 4KiB   [32x4]", &CopyNonTemporal32x4, KiB(4),     MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 8KiB   [32x4]", &CopyNonTemporal32x4, KiB(8),     MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 16KiB  [32x4]", &CopyNonTemporal32x4, KiB(16),    MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 32KiB  [32x4]", &CopyNonTemporal32x4, KiB(32),    MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 64KiB  [32x4]", &CopyNonTemporal32x4, KiB(64),    MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 128KiB [32x4]", &CopyNonTemporal32x4, KiB(128),   MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 256KiB [32x4]", &CopyNonTemporal32x4, KiB(256),   MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 512KiB [32x4]", &CopyNonTemporal32x4, KiB(512),   MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 1MiB   [32x4]", &CopyNonTemporal32x4, MiB(1),     MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 2MiB   [32x4]", &CopyNonTemporal32x4, MiB(2),     MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 4MiB   [32x4]", &CopyNonTemporal32x4, MiB(4),     MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 8MiB   [32x4]", &CopyNonTemporal32x4, MiB(8),     MemoryType_BAR, TestType_Copy },
    { "Copy Non-Temporal 16MiB  [32x4]", &CopyNonTemporal32x4, MiB(16),    MemoryType_BAR, TestType_Copy },
    //{ "Copy Non-Temporal 32MiB  [32x4]", &CopyNonTemporal32x4, MiB(32),    MemoryType_BAR, TestType_Copy },
    //{ "Copy Non-Temporal 64MiB  [32x4]", &CopyNonTemporal32x4, MiB(64),    MemoryType_BAR, TestType_Copy },
#endif

#if TestCopyTemporal
    { "Copy Temporal 4KiB   [32x4]", &Copy32x4, KiB(4),     MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 8KiB   [32x4]", &Copy32x4, KiB(8),     MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 16KiB  [32x4]", &Copy32x4, KiB(16),    MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 32KiB  [32x4]", &Copy32x4, KiB(32),    MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 64KiB  [32x4]", &Copy32x4, KiB(64),    MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 128KiB [32x4]", &Copy32x4, KiB(128),   MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 256KiB [32x4]", &Copy32x4, KiB(256),   MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 512KiB [32x4]", &Copy32x4, KiB(512),   MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 1MiB   [32x4]", &Copy32x4, MiB(1),     MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 2MiB   [32x4]", &Copy32x4, MiB(2),     MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 4MiB   [32x4]", &Copy32x4, MiB(4),     MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 8MiB   [32x4]", &Copy32x4, MiB(8),     MemoryType_BAR, TestType_Copy },
    { "Copy Temporal 16MiB  [32x4]", &Copy32x4, MiB(16),    MemoryType_BAR, TestType_Copy },
    //{ "Copy Temporal 32MiB  [32x4]", &Copy32x4, MiB(32),    MemoryType_BAR, TestType_Copy },
    //{ "Copy Temporal 64MiB  [32x4]", &Copy32x4, MiB(64),    MemoryType_BAR, TestType_Copy },
#endif

#if TestWriteNonTemporal
    { "Mem NonTemporal 4KiB   [32x4]", &WriteNonTemporal32x4, KiB(4),     MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 8KiB   [32x4]", &WriteNonTemporal32x4, KiB(8),     MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 16KiB  [32x4]", &WriteNonTemporal32x4, KiB(16),    MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 32KiB  [32x4]", &WriteNonTemporal32x4, KiB(32),    MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 64KiB  [32x4]", &WriteNonTemporal32x4, KiB(64),    MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 128KiB [32x4]", &WriteNonTemporal32x4, KiB(128),   MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 256KiB [32x4]", &WriteNonTemporal32x4, KiB(256),   MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 512KiB [32x4]", &WriteNonTemporal32x4, KiB(512),   MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 1MiB   [32x4]", &WriteNonTemporal32x4, MiB(1),     MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 2MiB   [32x4]", &WriteNonTemporal32x4, MiB(2),     MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 4MiB   [32x4]", &WriteNonTemporal32x4, MiB(4),     MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 8MiB   [32x4]", &WriteNonTemporal32x4, MiB(8),     MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 16MiB  [32x4]", &WriteNonTemporal32x4, MiB(16),    MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 32MiB  [32x4]", &WriteNonTemporal32x4, MiB(32),    MemoryType_Host, TestType_Write },
    { "Mem NonTemporal 64MiB  [32x4]", &WriteNonTemporal32x4, MiB(64),    MemoryType_Host, TestType_Write },
    { "Bar NonTemporal 4KiB   [32x4]", &WriteNonTemporal32x4, KiB(4),     MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 8KiB   [32x4]", &WriteNonTemporal32x4, KiB(8),     MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 16KiB  [32x4]", &WriteNonTemporal32x4, KiB(16),    MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 32KiB  [32x4]", &WriteNonTemporal32x4, KiB(32),    MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 64KiB  [32x4]", &WriteNonTemporal32x4, KiB(64),    MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 128KiB [32x4]", &WriteNonTemporal32x4, KiB(128),   MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 256KiB [32x4]", &WriteNonTemporal32x4, KiB(256),   MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 512KiB [32x4]", &WriteNonTemporal32x4, KiB(512),   MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 1MiB   [32x4]", &WriteNonTemporal32x4, MiB(1),     MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 2MiB   [32x4]", &WriteNonTemporal32x4, MiB(2),     MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 4MiB   [32x4]", &WriteNonTemporal32x4, MiB(4),     MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 8MiB   [32x4]", &WriteNonTemporal32x4, MiB(8),     MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 16MiB  [32x4]", &WriteNonTemporal32x4, MiB(16),    MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 32MiB  [32x4]", &WriteNonTemporal32x4, MiB(32),    MemoryType_BAR, TestType_Write },
    { "Bar NonTemporal 64MiB  [32x4]", &WriteNonTemporal32x4, MiB(64),    MemoryType_BAR, TestType_Write },
#endif

#if TestWriteTemporal
    { "Mem 4KiB   [32x4]", &Write32x4, KiB(4),     MemoryType_Host, TestType_Write },
    { "Mem 8KiB   [32x4]", &Write32x4, KiB(8),     MemoryType_Host, TestType_Write },
    { "Mem 16KiB  [32x4]", &Write32x4, KiB(16),    MemoryType_Host, TestType_Write },
    { "Mem 32KiB  [32x4]", &Write32x4, KiB(32),    MemoryType_Host, TestType_Write },
    { "Mem 64KiB  [32x4]", &Write32x4, KiB(64),    MemoryType_Host, TestType_Write },
    { "Mem 128KiB [32x4]", &Write32x4, KiB(128),   MemoryType_Host, TestType_Write },
    { "Mem 256KiB [32x4]", &Write32x4, KiB(256),   MemoryType_Host, TestType_Write },
    { "Mem 512KiB [32x4]", &Write32x4, KiB(512),   MemoryType_Host, TestType_Write },
    { "Mem 1MiB   [32x4]", &Write32x4, MiB(1),     MemoryType_Host, TestType_Write },
    { "Mem 2MiB   [32x4]", &Write32x4, MiB(2),     MemoryType_Host, TestType_Write },
    { "Mem 4MiB   [32x4]", &Write32x4, MiB(4),     MemoryType_Host, TestType_Write },
    { "Mem 8MiB   [32x4]", &Write32x4, MiB(8),     MemoryType_Host, TestType_Write },
    { "Mem 16MiB  [32x4]", &Write32x4, MiB(16),    MemoryType_Host, TestType_Write },
    { "Mem 32MiB  [32x4]", &Write32x4, MiB(32),    MemoryType_Host, TestType_Write },
    { "Mem 64MiB  [32x4]", &Write32x4, MiB(64),    MemoryType_Host, TestType_Write },
    { "Bar 4KiB   [32x4]", &Write32x4, KiB(4),     MemoryType_BAR, TestType_Write },
    { "Bar 8KiB   [32x4]", &Write32x4, KiB(8),     MemoryType_BAR, TestType_Write },
    { "Bar 16KiB  [32x4]", &Write32x4, KiB(16),    MemoryType_BAR, TestType_Write },
    { "Bar 32KiB  [32x4]", &Write32x4, KiB(32),    MemoryType_BAR, TestType_Write },
    { "Bar 64KiB  [32x4]", &Write32x4, KiB(64),    MemoryType_BAR, TestType_Write },
    { "Bar 128KiB [32x4]", &Write32x4, KiB(128),   MemoryType_BAR, TestType_Write },
    { "Bar 256KiB [32x4]", &Write32x4, KiB(256),   MemoryType_BAR, TestType_Write },
    { "Bar 512KiB [32x4]", &Write32x4, KiB(512),   MemoryType_BAR, TestType_Write },
    { "Bar 1MiB   [32x4]", &Write32x4, MiB(1),     MemoryType_BAR, TestType_Write },
    { "Bar 2MiB   [32x4]", &Write32x4, MiB(2),     MemoryType_BAR, TestType_Write },
    { "Bar 4MiB   [32x4]", &Write32x4, MiB(4),     MemoryType_BAR, TestType_Write },
    { "Bar 8MiB   [32x4]", &Write32x4, MiB(8),     MemoryType_BAR, TestType_Write },
    { "Bar 16MiB  [32x4]", &Write32x4, MiB(16),    MemoryType_BAR, TestType_Write },
    { "Bar 32MiB  [32x4]", &Write32x4, MiB(32),    MemoryType_BAR, TestType_Write },
    { "Bar 64MiB  [32x4]", &Write32x4, MiB(64),    MemoryType_BAR, TestType_Write },
#endif

    // Unused
#if 0
    { "Mem 4KiB   [32x2]", &Write32x2, KiB(4),     MemoryType_Host, TestType_Write },
    { "Mem 8KiB   [32x2]", &Write32x2, KiB(8),     MemoryType_Host, TestType_Write },
    { "Mem 16KiB  [32x2]", &Write32x2, KiB(16),    MemoryType_Host, TestType_Write },
    { "Mem 32KiB  [32x2]", &Write32x2, KiB(32),    MemoryType_Host, TestType_Write },
    { "Mem 64KiB  [32x2]", &Write32x2, KiB(64),    MemoryType_Host, TestType_Write },
    { "Mem 128KiB [32x2]", &Write32x2, KiB(128),   MemoryType_Host, TestType_Write },
    { "Mem 256KiB [32x2]", &Write32x2, KiB(256),   MemoryType_Host, TestType_Write },
    { "Mem 512KiB [32x2]", &Write32x2, KiB(512),   MemoryType_Host, TestType_Write },
    { "Mem 1MiB   [32x2]", &Write32x2, MiB(1),     MemoryType_Host, TestType_Write },
    { "Mem 2MiB   [32x2]", &Write32x2, MiB(2),     MemoryType_Host, TestType_Write },
    { "Mem 4MiB   [32x2]", &Write32x2, MiB(4),     MemoryType_Host, TestType_Write },
    { "Mem 8MiB   [32x2]", &Write32x2, MiB(8),     MemoryType_Host, TestType_Write },
    { "Mem 16MiB  [32x2]", &Write32x2, MiB(16),    MemoryType_Host, TestType_Write },
    { "Mem 32MiB  [32x2]", &Write32x2, MiB(32),    MemoryType_Host, TestType_Write },
    { "Mem 64MiB  [32x2]", &Write32x2, MiB(64),    MemoryType_Host, TestType_Write },
    { "Bar 4KiB   [32x2]", &Write32x2, KiB(4),     MemoryType_BAR, TestType_Write },
    { "Bar 8KiB   [32x2]", &Write32x2, KiB(8),     MemoryType_BAR, TestType_Write },
    { "Bar 16KiB  [32x2]", &Write32x2, KiB(16),    MemoryType_BAR, TestType_Write },
    { "Bar 32KiB  [32x2]", &Write32x2, KiB(32),    MemoryType_BAR, TestType_Write },
    { "Bar 64KiB  [32x2]", &Write32x2, KiB(64),    MemoryType_BAR, TestType_Write },
    { "Bar 128KiB [32x2]", &Write32x2, KiB(128),   MemoryType_BAR, TestType_Write },
    { "Bar 256KiB [32x2]", &Write32x2, KiB(256),   MemoryType_BAR, TestType_Write },
    { "Bar 512KiB [32x2]", &Write32x2, KiB(512),   MemoryType_BAR, TestType_Write },
    { "Bar 1MiB   [32x2]", &Write32x2, MiB(1),     MemoryType_BAR, TestType_Write },
    { "Bar 2MiB   [32x2]", &Write32x2, MiB(2),     MemoryType_BAR, TestType_Write },
    { "Bar 4MiB   [32x2]", &Write32x2, MiB(4),     MemoryType_BAR, TestType_Write },
    { "Bar 8MiB   [32x2]", &Write32x2, MiB(8),     MemoryType_BAR, TestType_Write },
    { "Bar 16MiB  [32x2]", &Write32x2, MiB(16),    MemoryType_BAR, TestType_Write },
    { "Bar 32MiB  [32x2]", &Write32x2, MiB(32),    MemoryType_BAR, TestType_Write },
    { "Bar 64MiB  [32x2]", &Write32x2, MiB(64),    MemoryType_BAR, TestType_Write },

    { "Mem 4KiB   [32x1]", &Write32x1, KiB(4),     MemoryType_Host, TestType_Write },
    { "Mem 8KiB   [32x1]", &Write32x1, KiB(8),     MemoryType_Host, TestType_Write },
    { "Mem 16KiB  [32x1]", &Write32x1, KiB(16),    MemoryType_Host, TestType_Write },
    { "Mem 32KiB  [32x1]", &Write32x1, KiB(32),    MemoryType_Host, TestType_Write },
    { "Mem 64KiB  [32x1]", &Write32x1, KiB(64),    MemoryType_Host, TestType_Write },
    { "Mem 128KiB [32x1]", &Write32x1, KiB(128),   MemoryType_Host, TestType_Write },
    { "Mem 256KiB [32x1]", &Write32x1, KiB(256),   MemoryType_Host, TestType_Write },
    { "Mem 512KiB [32x1]", &Write32x1, KiB(512),   MemoryType_Host, TestType_Write },
    { "Mem 1MiB   [32x1]", &Write32x1, MiB(1),     MemoryType_Host, TestType_Write },
    { "Mem 2MiB   [32x1]", &Write32x1, MiB(2),     MemoryType_Host, TestType_Write },
    { "Mem 4MiB   [32x1]", &Write32x1, MiB(4),     MemoryType_Host, TestType_Write },
    { "Mem 8MiB   [32x1]", &Write32x1, MiB(8),     MemoryType_Host, TestType_Write },
    { "Mem 16MiB  [32x1]", &Write32x1, MiB(16),    MemoryType_Host, TestType_Write },
    { "Mem 32MiB  [32x1]", &Write32x1, MiB(32),    MemoryType_Host, TestType_Write },
    { "Mem 64MiB  [32x1]", &Write32x1, MiB(64),    MemoryType_Host, TestType_Write },
    { "Bar 4KiB   [32x1]", &Write32x1, KiB(4),     MemoryType_BAR, TestType_Write },
    { "Bar 8KiB   [32x1]", &Write32x1, KiB(8),     MemoryType_BAR, TestType_Write },
    { "Bar 16KiB  [32x1]", &Write32x1, KiB(16),    MemoryType_BAR, TestType_Write },
    { "Bar 32KiB  [32x1]", &Write32x1, KiB(32),    MemoryType_BAR, TestType_Write },
    { "Bar 64KiB  [32x1]", &Write32x1, KiB(64),    MemoryType_BAR, TestType_Write },
    { "Bar 128KiB [32x1]", &Write32x1, KiB(128),   MemoryType_BAR, TestType_Write },
    { "Bar 256KiB [32x1]", &Write32x1, KiB(256),   MemoryType_BAR, TestType_Write },
    { "Bar 512KiB [32x1]", &Write32x1, KiB(512),   MemoryType_BAR, TestType_Write },
    { "Bar 1MiB   [32x1]", &Write32x1, MiB(1),     MemoryType_BAR, TestType_Write },
    { "Bar 2MiB   [32x1]", &Write32x1, MiB(2),     MemoryType_BAR, TestType_Write },
    { "Bar 4MiB   [32x1]", &Write32x1, MiB(4),     MemoryType_BAR, TestType_Write },
    { "Bar 8MiB   [32x1]", &Write32x1, MiB(8),     MemoryType_BAR, TestType_Write },
    { "Bar 16MiB  [32x1]", &Write32x1, MiB(16),    MemoryType_BAR, TestType_Write },
    { "Bar 32MiB  [32x1]", &Write32x1, MiB(32),    MemoryType_BAR, TestType_Write },
    { "Bar 64MiB  [32x1]", &Write32x1, MiB(64),    MemoryType_BAR, TestType_Write },
#endif
};

static test_context Initialize(void);

int main(void)
{
    test_context Context = Initialize();
    b32 AllBuffersPresent = 1;
    for (u32 MemoryType = 0; MemoryType < MemoryType_Count; MemoryType++)
    {
        if (!Context.Buffers[MemoryType])
        {
            AllBuffersPresent = 0;
            break;
        }
    }

    if (AllBuffersPresent)
    {
        printf("Device: %s\n", Context.DeviceName);
        printf("Frequency estimate: %f Ghz\n", Context.TSCFrequencyEstimate / (1000.0 * 1000.0 * 1000.0));

        //for (;;)
        {
            for (u32 TestIndex = 0; TestIndex < CountOf(Tests); TestIndex++)
            {
                test_config* Test = Tests + TestIndex;
                RunTest(&Context, Test);
            }
            printf("- - - - - - - - - - - - - - - - -\n");
        }
    }
    else
    {
        fprintf(stderr, "Failed to initialize");
    }
    return(0);
}

//
// Scaffolding
//

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

typedef struct VkInstance_T*        VkInstance;
typedef struct VkPhysicalDevice_T*  VkPhysicalDevice;
typedef struct VkDevice_T*          VkDevice;
typedef struct VkDeviceMemory_T*    VkDeviceMemory;

typedef enum VkResult
{
    VK_SUCCESS = 0,

    VK_RESULT_MAX_ENUM = 0x7FFFFFFF,
} VkResult;

typedef enum VkStructureType
{
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO      = 1,
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO  = 2,
    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO        = 3,
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO      = 5,

    VK_STRUCTURE_TYPE_MAX_ENUM                  = 0x7FFFFFFF,
} VkStructureType;

typedef struct VkInstanceCreateInfo
{
    VkStructureType                     sType;
    const void*                         pNext;
    flags32                             flags;
    const struct VkApplicationInfo*     pApplicationInfo;
    u32                                 enabledLayoutCount;
    const char* const*                  ppEnabledLayerNames;
    u32                                 enabledExtensionCount;
    const char* const*                  ppEnabledExtensionNames;

} VkInstanceCreateInfo;

typedef enum VkPhysicalDeviceType
{
    VK_PHYSICAL_DEVICE_TYPE_OTHER           = 0,
    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU  = 1,
    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU    = 2,
    VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU     = 3,
    VK_PHYSICAL_DEVICE_TYPE_CPU             = 4,

    VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM        = 0x7FFFFFFF,
} VkPhysicalDeviceType;

typedef struct VkPhysicalDeviceLimits
{
    u32     maxImageDimension1D;
    u32     maxImageDimension2D;
    u32     maxImageDimension3D;
    u32     maxImageDimensionCube;
    u32     maxImageArrayLayers;
    u32     maxTexelBufferElements;
    u32     maxUniformBufferRange;
    u32     maxStorageBufferRange;
    u32     maxPushConstantsSize;
    u32     maxMemoryAllocationCount;
    u32     maxSamplerAllocationCount;
    u64     bufferImageGranularity;
    u64     sparseAddressSpaceSize;
    u32     maxBoundDescriptorSets;
    u32     maxPerStageDescriptorSamplers;
    u32     maxPerStageDescriptorUniformBuffers;
    u32     maxPerStageDescriptorStorageBuffers;
    u32     maxPerStageDescriptorSampledImages;
    u32     maxPerStageDescriptorStorageImages;
    u32     maxPerStageDescriptorInputAttachments;
    u32     maxPerStageResources;
    u32     maxDescriptorSetSamplers;
    u32     maxDescriptorSetUniformBuffers;
    u32     maxDescriptorSetUniformBuffersDynamic;
    u32     maxDescriptorSetStorageBuffers;
    u32     maxDescriptorSetStorageBuffersDynamic;
    u32     maxDescriptorSetSampledImages;
    u32     maxDescriptorSetStorageImages;
    u32     maxDescriptorSetInputAttachments;
    u32     maxVertexInputAttributes;
    u32     maxVertexInputBindings;
    u32     maxVertexInputAttributeOffset;
    u32     maxVertexInputBindingStride;
    u32     maxVertexOutputComponents;
    u32     maxTessellationGenerationLevel;
    u32     maxTessellationPatchSize;
    u32     maxTessellationControlPerVertexInputComponents;
    u32     maxTessellationControlPerVertexOutputComponents;
    u32     maxTessellationControlPerPatchOutputComponents;
    u32     maxTessellationControlTotalOutputComponents;
    u32     maxTessellationEvaluationInputComponents;
    u32     maxTessellationEvaluationOutputComponents;
    u32     maxGeometryShaderInvocations;
    u32     maxGeometryInputComponents;
    u32     maxGeometryOutputComponents;
    u32     maxGeometryOutputVertices;
    u32     maxGeometryTotalOutputComponents;
    u32     maxFragmentInputComponents;
    u32     maxFragmentOutputAttachments;
    u32     maxFragmentDualSrcAttachments;
    u32     maxFragmentCombinedOutputResources;
    u32     maxComputeSharedMemorySize;
    u32     maxComputeWorkGroupCount[3];
    u32     maxComputeWorkGroupInvocations;
    u32     maxComputeWorkGroupSize[3];
    u32     subPixelPrecisionBits;
    u32     subTexelPrecisionBits;
    u32     mipmapPrecisionBits;
    u32     maxDrawIndexedIndexValue;
    u32     maxDrawIndirectCount;
    f32     maxSamplerLodBias;
    f32     maxSamplerAnisotropy;
    u32     maxViewports;
    u32     maxViewportDimensions[2];
    f32     viewportBoundsRange[2];
    u32     viewportSubPixelBits;
    umm     minMemoryMapAlignment;
    u64     minTexelBufferOffsetAlignment;
    u64     minUniformBufferOffsetAlignment;
    u64     minStorageBufferOffsetAlignment;
    s32     minTexelOffset;
    u32     maxTexelOffset;
    s32     minTexelGatherOffset;
    u32     maxTexelGatherOffset;
    f32     minInterpolationOffset;
    f32     maxInterpolationOffset;
    u32     subPixelInterpolationOffsetBits;
    u32     maxFramebufferWidth;
    u32     maxFramebufferHeight;
    u32     maxFramebufferLayers;
    flags32 framebufferColorSampleCounts;
    flags32 framebufferDepthSampleCounts;
    flags32 framebufferStencilSampleCounts;
    flags32 framebufferNoAttachmentsSampleCounts;
    u32     maxColorAttachments;
    flags32 sampledImageColorSampleCounts;
    flags32 sampledImageIntegerSampleCounts;
    flags32 sampledImageDepthSampleCounts;
    flags32 sampledImageStencilSampleCounts;
    flags32 storageImageSampleCounts;
    u32     maxSampleMaskWords;
    b32     timestampComputeAndGraphics;
    f32     timestampPeriod;
    u32     maxClipDistances;
    u32     maxCullDistances;
    u32     maxCombinedClipAndCullDistances;
    u32     discreteQueuePriorities;
    f32     pointSizeRange[2];
    f32     lineWidthRange[2];
    f32     pointSizeGranularity;
    f32     lineWidthGranularity;
    b32     strictLines;
    b32     standardSampleLocations;
    u64     optimalBufferCopyOffsetAlignment;
    u64     optimalBufferCopyRowPitchAlignment;
    u64     nonCoherentAtomSize;
} VkPhysicalDeviceLimits;

typedef struct VkPhysicalDeviceSparseProperties
{
    b32 residencyStandard2DBlockShape;
    b32 residencyStandard2DMultisampleBlockShape;
    b32 residencyStandard3DBlockShape;
    b32 residencyAlignedMipSize;
    b32 residencyNonResidentStrict;
} VkPhysicalDeviceSparseProperties;

typedef struct VkPhysicalDeviceProperties
{
    u32                                 apiVersion;
    u32                                 driverVersion;
    u32                                 vendorID;
    u32                                 deviceID;
    VkPhysicalDeviceType                deviceType;
    char                                deviceName[256];
    __m128i                             pipelineCacheUUID;
    VkPhysicalDeviceLimits              limits;
    VkPhysicalDeviceSparseProperties    sparseProperties;
} VkPhysicalDeviceProperties;

#define VK_MAX_MEMORY_TYPES 32u
#define VK_MAX_MEMORY_HEAPS 16u

typedef enum VkMemoryPropertyFlagBits
{
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT     = 0x01,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT     = 0x02,
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT    = 0x04,
    VK_MEMORY_PROPERTY_HOST_CACHED_BIT      = 0x08,

    VK_MEMORY_PROPERTY_MAX_ENUM             = 0x7FFFFFFF,
} VkMemoryPropertyFlagBits;

typedef struct VkMemoryType
{
    flags32 propertyFlags;
    u32     heapIndex;
} VkMemoryType;

typedef struct VkMemoryHeap
{
    u64             size;
    flags32         flags;
} VkMemoryHeap;

typedef struct VkPhysicalDeviceMemoryProperties
{
    u32             memoryTypeCount;
    VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES];
    u32             memoryHeapCount;
    VkMemoryHeap    memoryHeaps[VK_MAX_MEMORY_HEAPS];
} VkPhysicalDeviceMemoryProperties;

typedef struct VkDeviceQueueCreateInfo
{
    VkStructureType     sType;
    const void*         pNext;
    flags32             flags;
    u32                 queueFamilyIndex;
    u32                 queueCount;
    const float*        pQueuePriorities;
} VkDeviceQueueCreateInfo;

typedef struct VkDeviceCreateInfo
{
    VkStructureType                         sType;
    const void*                             pNext;
    flags32                                 flags;
    u32                                     queueCreateInfoCount;
    const VkDeviceQueueCreateInfo*          pQueueCreateInfos;
    u32                                     enabledLayerCount;
    const char* const*                      ppEnabledLayerNames;
    u32                                     enabledExtensionCount;
    const char* const*                      ppEnabledExtensionNames;
    const struct VkPhysicalDeviceFeatures*  pEnabledFeatures;
} VkDeviceCreateInfo;

typedef struct VkMemoryAllocateInfo
{
    VkStructureType     sType;
    const void*         pNext;
    u64                 allocationSize;
    u32                 memoryTypeIndex;
} VkMemoryAllocateInfo;

typedef void*       (__stdcall * PFN_vkGetInstanceProcAddr)                 (VkInstance, const char*);
typedef void*       (__stdcall * PFN_vkGetDeviceProcAddr)                   (VkDevice, const char*);
typedef VkResult    (__stdcall * PFN_vkCreateInstance)                      (const VkInstanceCreateInfo*, const struct VkAllocationCallbacks*, VkInstance*);
typedef VkResult    (__stdcall * PFN_vkEnumeratePhysicalDevices)            (VkInstance, u32*, VkPhysicalDevice*);
typedef void        (__stdcall * PFN_vkGetPhysicalDeviceProperties)         (VkPhysicalDevice, VkPhysicalDeviceProperties*);
typedef void        (__stdcall * PFN_vkGetPhysicalDeviceMemoryProperties)   (VkPhysicalDevice, VkPhysicalDeviceMemoryProperties*);
typedef VkResult    (__stdcall * PFN_vkCreateDevice)                        (VkPhysicalDevice, const VkDeviceCreateInfo*, const struct VkAllocationCallbacks*, VkDevice*);
typedef VkResult    (__stdcall * PFN_vkAllocateMemory)                      (VkDevice, const VkMemoryAllocateInfo*, const struct VkAllocationCallbacks*, VkDeviceMemory*);
typedef VkResult    (__stdcall * PFN_vkMapMemory)                           (VkDevice, VkDeviceMemory, u64, u64, flags32, void**);

#define LoadFunctionPointer(loader, handle, name) PFN_##name name = (PFN_##name)loader(handle, #name)

static test_context Initialize(void)
{
    test_context Context = {0};

    // Estimate TSC frequency
    {

        u64 Begin = __rdtsc();
        LARGE_INTEGER QPCFreq;
        QueryPerformanceFrequency(&QPCFreq);
        LARGE_INTEGER QPCBegin;
        QueryPerformanceCounter(&QPCBegin);
        for (;;)
        {
            LARGE_INTEGER QPCEnd;
            QueryPerformanceCounter(&QPCEnd);

            if ((QPCEnd.QuadPart - QPCBegin.QuadPart) >= QPCFreq.QuadPart)
            {
                break;
            }
        }
        u64 End = __rdtsc();

        Context.TSCFrequencyEstimate = End - Begin;
    }

    HMODULE VulkanDLL = LoadLibraryA("vulkan-1.dll");
    if (VulkanDLL)
    {
        LoadFunctionPointer(GetProcAddress, VulkanDLL, vkGetInstanceProcAddr);

        if (vkGetInstanceProcAddr)
        {
            LoadFunctionPointer(vkGetInstanceProcAddr, 0, vkCreateInstance);

            VkInstanceCreateInfo InstanceInfo = 
            {
                .sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext                      = 0,
                .flags                      = 0,
                .pApplicationInfo           = 0,
                .enabledLayoutCount         = 0,
                .ppEnabledLayerNames        = 0,
                .enabledExtensionCount      = 0,
                .ppEnabledExtensionNames    = 0,
            };

            VkInstance Instance = 0;
            VkResult Result = vkCreateInstance(&InstanceInfo, 0, &Instance);
            if (Result == VK_SUCCESS)
            {
                LoadFunctionPointer(vkGetInstanceProcAddr, Instance, vkGetDeviceProcAddr);
                LoadFunctionPointer(vkGetInstanceProcAddr, Instance, vkEnumeratePhysicalDevices);
                LoadFunctionPointer(vkGetInstanceProcAddr, Instance, vkGetPhysicalDeviceProperties);
                LoadFunctionPointer(vkGetInstanceProcAddr, Instance, vkGetPhysicalDeviceMemoryProperties);
                LoadFunctionPointer(vkGetInstanceProcAddr, Instance, vkCreateDevice);

                VkPhysicalDeviceProperties          Props;
                VkPhysicalDeviceMemoryProperties    MemoryProps;
                VkPhysicalDevice                    Devices[8];
                u32                                 DeviceCount         = CountOf(Devices);
                VkPhysicalDevice                    SelectedDevice      = 0;
                u32                                 SelectedMemoryType  = 0;

                vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices);
                for (u32 DeviceIndex = 0; DeviceIndex < DeviceCount; DeviceIndex++)
                {
                    vkGetPhysicalDeviceProperties(Devices[DeviceIndex], &Props);
                    if (Props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                    {
                        vkGetPhysicalDeviceMemoryProperties(Devices[DeviceIndex], &MemoryProps);
                        for (u32 MemoryTypeIndex = 0; MemoryTypeIndex < MemoryProps.memoryTypeCount; MemoryTypeIndex++)
                        {
                            VkMemoryType* Type = MemoryProps.memoryTypes + MemoryTypeIndex;
                            flags32 Flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
                            if ((Type->propertyFlags & Flags) == Flags && !(Type->propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT))
                            {
                                SelectedDevice = Devices[DeviceIndex];
                                SelectedMemoryType = MemoryTypeIndex;
                                break;
                            }
                        }
                    }

                    if (SelectedDevice)
                    {
                        break;
                    }
                }

                if (SelectedDevice)
                {
                    VkDeviceCreateInfo DeviceInfo = 
                    {
                        .sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                        .pNext                      = 0,
                        .queueCreateInfoCount       = 1,
                        .pQueueCreateInfos = &(VkDeviceQueueCreateInfo)
                        {
                            .sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                            .pNext              = 0,
                            .flags              = 0,
                            .queueFamilyIndex   = 0,
                            .queueCount         = 1,
                            .pQueuePriorities   = (float[]){ 0.0f },
                        },
                        .enabledLayerCount          = 0,
                        .ppEnabledLayerNames        = 0,
                        .enabledExtensionCount      = 0,
                        .ppEnabledExtensionNames    = 0,
                        .pEnabledFeatures           = 0,
                    };
                    VkDevice Device = 0;
                    Result = vkCreateDevice(SelectedDevice, &DeviceInfo, 0, &Device);
                    if (Result == VK_SUCCESS)
                    {
                        LoadFunctionPointer(vkGetDeviceProcAddr, Device, vkAllocateMemory);
                        LoadFunctionPointer(vkGetDeviceProcAddr, Device, vkMapMemory);

                        VkMemoryAllocateInfo AllocInfo = 
                        {
                            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                            .pNext = 0,
                            .allocationSize = 64llu << 20,
                            .memoryTypeIndex = SelectedMemoryType,
                        };

                        VkDeviceMemory Memory = 0;
                        Result = vkAllocateMemory(Device, &AllocInfo, 0, &Memory);
                        if (Result == VK_SUCCESS)
                        {
                            void* Mapping = 0;
                            if (vkMapMemory(Device, Memory, 0, ~(0llu), 0, &Mapping) == VK_SUCCESS)
                            {
                                Context.BufferSize = AllocInfo.allocationSize;
                                Context.Buffers[MemoryType_BAR] = Mapping;
                                memcpy(Context.DeviceName, Props.deviceName, sizeof(Context.DeviceName));
                            }
                        }
                    }
                }
            }
        }
    }

    Context.Buffers[MemoryType_Host] = VirtualAlloc(0, Context.BufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    return(Context);
}
