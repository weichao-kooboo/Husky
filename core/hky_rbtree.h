#ifndef HKY_RBTREE_H_INCLUDED
#define HKY_RBTREE_H_INCLUDED

#include "hky_config.h"
#include "hky_core.h"

typedef hky_uint_t hky_rbtree_key_t;
typedef hky_int_t  hky_rbtree_key_int_t;

typedef struct hky_rbtree_node_s hky_rbtree_node_t;

struct hky_rbtree_node_s{
    /*键值*/
    hky_rbtree_key_t key;
    /*左子树*/
    hky_rbtree_node_t *left;
    /*右子树*/
    hky_rbtree_node_t *right;
    /*父节点*/
    hky_rbtree_node_t *parent;
    /*rb值*/
    hky_uchar color;
    /*数据*/
    hky_uchar data;
};

typedef struct hky_rbtree_s hky_rbtree_t;
//函数指针
typedef void (*hky_rbtree_insert_pt)(hky_rbtree_node_t *root,hky_rbtree_node_t *node,hky_rbtree_node_t *sentinel);

struct hky_rbtree_s{
    /*根节点*/
    hky_rbtree_node_t *root;
    /*哨兵节点,用于标记遍历结束的位置,一般设置为NULL*/
    hky_rbtree_node_t *sentinel;
    /*插入树的回调函数*/
    hky_rbtree_insert_pt insert;
};

#define hky_rbtree_init(tree,s,i)       \
    hky_rbtree_sentinel_init(s);            \
    (tree)->root=s;                                     \
    (tree)->sentinel=s;                             \
    (tree)->insert=i
/*
* rbtree插入节点,然后保持树的平衡
* tree:rbtree结构
* node:待插入的节点
*/
void hky_rbtree_insert(hky_rbtree_t *tree,hky_rbtree_node_t *node);
/*
* rbtree删除节点,然后保持树的平衡
* tree:rbtree结构
* node:待删除的节点
*/
void hky_rbtree_delete(hky_rbtree_t *tree,hky_rbtree_node_t *node);
/*
* ???
*/
void hky_rbtree_insert_value(hky_rbtree_node_t *root, hky_rbtree_node_t *node,
    hky_rbtree_node_t *sentinel);
/*
* ???
*/
void  hky_rbtree_insert_timer_value(hky_rbtree_node_t *root,hky_rbtree_node_t *node,hky_rbtree_node_t *sentinel);
/*
* 找到rbtree指定节点的下一个节点
* tree:rbtree结构
* node:指定节点
*/
hky_rbtree_node_t *hky_rbtree_next(hky_rbtree_t *tree,hky_rbtree_node_t *node);

#define hky_rbt_red(node) ((node)->color=1)
#define hky_rbt_black(node) ((node)->color=0)
#define hky_rbt_is_red(node) ((node)->color)
#define hky_rbt_is_black(node) (!hky_rbt_is_red(node))
#define hky_rbt_copy_color(n1,n2) (n1->color=n2->color)
//哨兵节点必须是黑色
#define hky_rbtree_sentinel_init(node) hky_rbt_black(node)

static hky_inline hky_rbtree_node_t *
hky_rbtree_min(hky_rbtree_node_t *node,hky_rbtree_node_t *sentinel){
    while(node->left!=sentinel){
        node=node->left;
    }
    return node;
}

#endif // HKY_RBTREE_H_INCLUDED
