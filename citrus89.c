#include "citrus89.h"

DefineInterface(Allocator)
jmp_buf global_allocator_jump_buffer;

static Allocator global_allocator = NULL;
void SetGlobalAllocator(Allocator allocator) { global_allocator = allocator; }
void *Allocate(size_t num_bytes) {
  void *result;
  if (!global_allocator) global_allocator = MallocAllocator();

  result = AllocatorAllocate(global_allocator, num_bytes);
  if (!result)
    longjmp(global_allocator_jump_buffer, -1);
  return result;
}
void Free(void *allocation) {
  if (!global_allocator) global_allocator = MallocAllocator();
  AllocatorFree(global_allocator, allocation);
}
void ClearAllocator() {
  if (!global_allocator) global_allocator = MallocAllocator();
  AllocatorClear(global_allocator);
}

/* Malloc Allocator */
void *malloc(size_t num_bytes);
void free(void *data);
static void *MallocAllocate(void *object, size_t num_bytes) {
  object;
  return malloc(num_bytes);
}
static int MallocFree(void *object, void *data) {
  object;
  free(data);
  return 0;
}
static int MallocClear(void *object) { object; return 0; }
Allocator MallocAllocator() {
  static struct Allocator_MethodTable method_table = { MallocAllocate, MallocFree, MallocClear };
  static struct Allocator malloc_allocator = { &method_table, NULL, };
  return &malloc_allocator;
}

/* Pool */
struct PoolFreeList {
  PoolFreeList next;
};
DefineStruct(Pool)

Pool MakePool(size_t element_size, Allocator element_allocator) {
  return InitPool(element_size, element_allocator, StructAllocate(Pool));
}
Pool InitPool(size_t element_size, Allocator element_allocator, Pool pool) {
  return _InitPool(element_size, element_allocator, NULL, pool);
}

void *PoolAllocate(Pool pool) {
  PoolFreeList result = pool->next;
  if (result) {
    pool->next = result->next;
    return (void *)result;
  }
  return AllocatorAllocate(pool->element_allocator, pool->element_size);
}

void PoolFree(Pool pool, void *element) {
  PoolFreeList next = (PoolFreeList)element;
  next->next = pool->next;
  pool->next = next;
}

void PoolReset(Pool pool) {
  PoolFreeList next = pool->next;
  while (next) {
    PoolFreeList n = next->next;
    AllocatorFree(pool->element_allocator, next);
    next = n;
  }
  pool->next = NULL;
}

/*
Pool pool = MakePool(64, MallocAllocator());
int i;
void *ptrs[10];
for (i = 0; i < 10; ++i)
  printf("%x\n", ptrs[i]=PoolAllocate(pool));
for (i = 0; i < 10; ++i)
  PoolFree(pool, ptrs[i]);
PoolReset(pool);
printf("\n");
for (i = 0; i < 10; ++i)
  printf("%x\n", PoolAllocate(pool));
 */

/* Stack Allocator */

DefineStruct(Stack)

Stack MakeStack(u1 *buffer_bytes, size_t buffer_size) {
  return InitStack(buffer_bytes, buffer_size, StructAllocate(Stack));
}
Stack InitStack(u1 *buffer_bytes, size_t buffer_size, Stack stack) {
  return _InitStack(buffer_bytes, buffer_size, 0, stack);
}

u1 *StackAllocate(Stack stack, size_t num_bytes) {
  if (stack->buffer_size - stack->index < num_bytes) {
    return NULL;
  } else {
    u1 *result = &stack->buffer_bytes[stack->index];
    stack->index += num_bytes;
    return result;
  }
}

size_t Align(size_t index, size_t num_bytes) { 
  const size_t align = num_bytes - 1;
  return (index + align) & ~align;
}
/*
int i;
for (i = 0; i < 32; ++i)
  printf("index=%d aligned8=%d\n", i, Align(i, 8));
 */

void StackAlign(Stack stack, size_t num_bytes) {
  stack->index = Align(stack->index,num_bytes);
}

u1 *StackClear(Stack stack) {
  stack->index = 0;
}

/*
Stack stack = MakeStack(Allocate(64), 64);
int i;
for (i = 0; i < 20; ++i)
  printf("%d %x\n", i, StackAllocate(stack, 4));
*/

struct DynamicStackAllocationHeader {
  DynamicStackAllocationHeader next;
  b8 is_page;
};
DefineStruct(DynamicStack)

DynamicStack MakeDynamicStack(Pool page_pool) {
  return InitDynamicStack(page_pool, StructAllocate(Stack), StructAllocate(DynamicStack));
}
DynamicStack InitDynamicStack(Pool page_pool, Stack stack, DynamicStack dynamic_stack) {
  return _InitDynamicStack(page_pool, NULL, InitStack(NULL, 0, stack), dynamic_stack);
}

static void *DynamicStackAddHeader(DynamicStack dynamic_stack, void *allocation, b8 is_page) {
  if (!allocation) {
    /* Out of memory */
    return NULL;
  } else {
    DynamicStackAllocationHeader header = (DynamicStackAllocationHeader)allocation;

    /* Add the allocation to the allocation list */
    header->is_page = is_page;
    header->next = dynamic_stack->last_allocation;
    dynamic_stack->last_allocation = header;

    /* Return the allocation data */
    return (u1 *)header + Sizeof(DynamicStackAllocationHeader);
  }
}

void *DynamicStackAllocate(DynamicStack dynamic_stack, size_t num_bytes) {
  const size_t page_size = dynamic_stack->page_pool->element_size;
  const size_t header_size = Sizeof(DynamicStackAllocationHeader);
  const size_t stack_size = page_size - header_size;
  if (num_bytes <= stack_size) {
    /* Allocation fits in a single page */

    /* Attempt stack allocation */
    void *result = StackAllocate(dynamic_stack->stack, num_bytes);
    if (result) {
      return result;
    } else {
      /* Not enough room on the stack: Allocate another stack from the page_pool */
      void *page = DynamicStackAddHeader(dynamic_stack, PoolAllocate(dynamic_stack->page_pool), true);
      if (!page) return NULL; /* Out of memory */

      /* Set up the stack to point at the new page */
      InitStack((u1 *)page, stack_size, dynamic_stack->stack);

      /* New stack is guaranteed to have enough memory */
      return StackAllocate(dynamic_stack->stack, num_bytes);
    }
  } else {
    /* Allocation can't fit in a single page, use malloc */
    return DynamicStackAddHeader(dynamic_stack, malloc(num_bytes + header_size), false);
  }
}

void DynamicStackAlign(DynamicStack dynamic_stack, size_t num_bytes) {
  StackAlign(dynamic_stack->stack, num_bytes);
}

void DynamicStackClear(DynamicStack dynamic_stack) {
  /* Free all allocations */
  DynamicStackAllocationHeader header = dynamic_stack->last_allocation;
  while (header) {
    DynamicStackAllocationHeader next = header->next;
    /* If it is a page, return it to the page pool */
    if (header->is_page) PoolFree(dynamic_stack->page_pool, header);
    else free(header); /* Else free it */
    header = next;
  }
  /* Re-initialize the dynamic stack */
  InitDynamicStack(dynamic_stack->page_pool, dynamic_stack->stack, dynamic_stack);
}

Method(DynamicStack, Allocator, Allocate)
  return DynamicStackAllocate(self, num_bytes);
EndMethod
Method(DynamicStack, Allocator, Free)
  return 0;
EndMethod
Method(DynamicStack, Allocator, Clear)
  DynamicStackClear(self);
  return 0;
EndMethod

#define OBJECT DynamicStack
ObjectConstructors(Allocator)
#undef OBJECT

/*

int i;
SetGlobalAllocator(DynamicStackAllocator(MakeDynamicStack(MakePool(64, MallocAllocator()))));
for (i = 0; i < 64; ++i)
  printf("%d, %x\n", i, Allocate(4));
printf("%x\n", Allocate(128));
printf("%d, %x\n", i, Allocate(128));
for (i = 0; i < 64; ++i)
  printf("%d, %x\n", i, Allocate(4));
ClearAllocator();
for (i = 0; i < 64; ++i)
  printf("%d, %x\n", i, Allocate(4));
printf("%x\n", Allocate(128));
printf("%d, %x\n", i, Allocate(128));
for (i = 0; i < 64; ++i)
  printf("%d, %x\n", i, Allocate(4));

 */

String VSPrintf(size_t *length, String format, va_list args) {
  static char buffer[SPRINTF_BUFFER_SIZE_IN_BYTES];

  MString result;
  /* NOTE: Comes from C11 */
  *length = vsnprintf(buffer, SPRINTF_BUFFER_SIZE_IN_BYTES, format, args);

  *length = *length <= SPRINTF_BUFFER_SIZE_IN_BYTES ? *length : SPRINTF_BUFFER_SIZE_IN_BYTES;

  result = Allocate(*length+1);
  memcpy(result, buffer, *length);
  result[*length] = '\0';

  return result;
}
String SPrintf(size_t *length, String format, ...) {
  MString result;
  va_list args;
  va_start(args, format);
  result = VSPrintf(length, format, args);
  va_end(args);
  return result;
}
/*
size_t length = 0;
printf("%s\n", SPrintf(&length, "<%d %d>", 3, 4));
printf("%d\n", length);
 */

DefineStruct(StringBuffer)
#define STRUCT_StringBufferString(f)\
  f(size_t, string_length)\
  f(MString, string)\
  f(StringBufferString, next_string)
DeclareStructType(STRUCT_StringBufferString, StringBufferString)
DefineStruct(StringBufferString)

StringBuffer MakeStringBuffer(Allocator allocator) {
  return InitStringBuffer(allocator, StructAllocate(StringBuffer));
}
StringBuffer InitStringBuffer(Allocator allocator, StringBuffer buffer) {
  return _InitStringBuffer(allocator, NULL, NULL, 0, buffer);
}

void StringBufferPrintf(StringBuffer string_buffer, String format, ...) {
  StringBufferString string = (StringBufferString)AllocatorAllocate(string_buffer->allocator, Sizeof(StringBufferString));
  va_list args;

  va_start(args, format);
  string->string = VSPrintf(&string->string_length, format, args);
  va_end(args);
  string->next_string = NULL;
  string_buffer->length += string->string_length;

  if (!string_buffer->first_string) {
    string_buffer->first_string = string_buffer->last_string = string;
  } else {
    string_buffer->last_string->next_string = string;
    string_buffer->last_string = string;
  }
}
 
String StringBufferFinalize(StringBuffer buffer) {
  MString result = Allocate(buffer->length + 1);
  size_t index = 0;
  StringBufferString string = buffer->first_string;
  while (string) {
    memcpy(&result[index], string->string, string->string_length);
    index += string->string_length;
    string = string->next_string;
  }
  result[buffer->length] = '\0';

  AllocatorClear(buffer->allocator);
  buffer->first_string = buffer->last_string = NULL;
  buffer->length = 0;

  return result;
}
/*
StringBuffer buffer = MakeStringBuffer(DynamicStackAllocator(MakeDynamicStack(MakePool(4096, MallocAllocator()))));
StringBufferPrintf(buffer, "Hello %s\n", "Chebert");
StringBufferPrintf(buffer, "<%d %d>", 3, 4);
StringBufferPrintf(buffer, "<%d %d>", 5, 8);
printf("%s\n", StringBufferFinalize(buffer));
 */
