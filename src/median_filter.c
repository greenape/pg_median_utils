#include "postgres.h"
#include "fmgr.h"
#include "windowapi.h"
#include "quick_select.h"


PG_FUNCTION_INFO_V1(median_filter);

Datum median_filter(PG_FUNCTION_ARGS) {
    WindowObject win_obj = PG_WINDOW_OBJECT();
    uint32_t row = WinGetCurrentPosition(win_obj);
    uint32_t j;
    double *med;
    double median;
    Datum value;
    bool isnull, isout;
    bool window_size_is_null;
    Datum window_size_datum = WinGetFuncArgCurrent(win_obj, 1, &window_size_is_null);
    uint32_t window_size = DatumGetInt32(window_size_datum);
    if (row == 0) {
        /* Validate input parameters */
        if (window_size_is_null || window_size % 2 == 0) {
            ereport(ERROR,
                    (
                            errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                                    errmsg("window size must be odd"),
                                    errdetail("value %d is even or null", window_size),
                                    errhint("make it odd")
                    )
            );
            PG_RETURN_NULL();
        }
    }

    uint32_t offset = (window_size - 1) / 2;
    med = palloc(window_size * sizeof(double));
    memset(med, 0.0, window_size * sizeof(double));
    for (j = 0; j < window_size; j++) {
        if (((row + j - offset) < WinGetPartitionRowCount(win_obj)) && ((row + j) >= offset)) {
            value = WinGetFuncArgInPartition(win_obj, 0, row + j - offset,
                                             WINDOW_SEEK_HEAD, false, &isnull, &isout);
            med[j] = DatumGetFloat8(value);
        }
    }

    median = quick_select(med, window_size);
    pfree(med);
    PG_RETURN_FLOAT8(median);
}