#ifndef _CAL_H_
#define _CAL_H_

#ifdef __cplusplus
extern "C" {
#endif

struct data;
struct cat_root;
struct item;

void cal_data_total(struct data *dt, struct cat_root *cat);
void update_item_cat(struct item *it, struct cat_root *cat);

#ifdef __cplusplus
}
#endif

#endif
