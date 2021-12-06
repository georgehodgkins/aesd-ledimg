#ifndef _LEDGRID_H_
#define _LEDGRID_H_

#ifdef __cplusplus
extern "C" {
#endif

int grid_init (void);
int grid_select (unsigned);
int grid_free (void);

#ifdef __cplusplus
}
#endif

#define LEDGRID_ROWS 4
#define LEDGRID_COLS 4
#endif // _LEDGRID_H_
