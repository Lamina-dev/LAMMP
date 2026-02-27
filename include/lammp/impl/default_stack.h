#ifndef __LAMMP_DEFAULT_STACK_H__
#define __LAMMP_DEFAULT_STACK_H__

#include <stdint.h>
#include <stddef.h>

/**
 * @brief 默认栈重置
 * @param size 新栈大小，单位为字节
 * @note 该函数会释放原有栈空间，并重新分配新栈空间，若size为0，则会仅释放原有栈空间
 * @warning 重置默认栈后，原有栈空间上的指针、引用等数据将会失效，访问之前分配的
 *          内存将会导致未定义行为
 */
void lmmp_default_stack_reset(size_t size);

/**
 * @brief 默认栈获取栈顶
 */
void* lmmp_default_stack_get_top(void);

/**
 * @brief 默认栈设置栈顶
 */
void lmmp_default_stack_set_top(void* top);

#endif // __LAMMP_DEFAULT_STACK_H__