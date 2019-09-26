#include <math.h>
#include "postgres.h"
#include "fmgr.h"
#include "windowapi.h"
#include "quick_select.h"


typedef struct {
    double median[1];
} rolling_median_impute_context;


PG_FUNCTION_INFO_V1(rolling_median_impute);

Datum rolling_median_impute(PG_FUNCTION_ARGS) {
    WindowObject win_obj = PG_WINDOW_OBJECT();
    uint32_t row = WinGetCurrentPosition(win_obj);
    uint32_t nelems = WinGetPartitionRowCount(win_obj);

    uint32_t i;
    uint32_t j;
    double *med;
    Datum value;
    bool isnull, isout;

    bool window_size_is_null;
    Datum window_size_datum = WinGetFuncArgCurrent(win_obj, 1, &window_size_is_null);
    uint32_t window_size = DatumGetInt32(window_size_datum);

    rolling_median_impute_context *context = (rolling_median_impute_context *) WinGetPartitionLocalMemory(win_obj,
                                                                                                          sizeof(rolling_median_impute_context) +
    nelems*sizeof(double));

    if (row == 0) /* beginning of the partition; do all of the work now */
    {
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

        if (window_size > nelems) {
            /* Can't do anything sensible here, so error. */
            ereport(ERROR,
                    (
                            errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                                    errmsg("window size greater than number of values"),
                                    errdetail("window size %d is greater than total number of elements in partition: %d", window_size, nelems),
                                    errhint("reduce window size")
                    )
            );
            PG_RETURN_NULL();
        }

        double *non_nulls = palloc(nelems*sizeof(double));
        double *medians = palloc(nelems*sizeof(double));
        int n_not_null = 0;
        /* Collect nonnull values */
        for (i = 0; i < nelems; i++) {
            value = WinGetFuncArgInPartition(win_obj, 0, i,
                                             WINDOW_SEEK_HEAD, false, &isnull, &isout);
            if (!isnull && !isout) {
                non_nulls[n_not_null] = DatumGetFloat8(value);
                n_not_null++;
            }
        }

        if (window_size > n_not_null) {
            /* Can't do anything sensible here, so error. */
            ereport(ERROR,
                    (
                            errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                                    errmsg("Too few non-nulls"),
                                    errdetail("window size %d is greater than total number of non-null elements in partition: %d", window_size, n_not_null),
                                    errhint("reduce window size")
                    )
            );
            PG_RETURN_NULL();
        }


        med = palloc(window_size * sizeof(double));
        for(i = window_size - 1; i < n_not_null; i++) {
            for(j = 0; j < window_size; j++) {
                med[j] = non_nulls[i - j];
            }
            medians[i] = quick_select(med, window_size);
        }
        /* Backfill first n */
        for(i = 0; i < window_size - 1; i++) {
            medians[i] = medians[window_size-1];
        }

        int median_ix = 0;
        int not_nulls;
        not_nulls = 0;
        for(i = 0; i < nelems; i++) {
            value = WinGetFuncArgInPartition(win_obj, 0, i,
                                             WINDOW_SEEK_HEAD, false, &isnull, &isout);
            if(!isnull) {
                not_nulls++;
            }
            if(!isnull && (median_ix < (n_not_null - 1))) {
                /* Move median ix on one */
                median_ix++;
            }
            context->median[i] = medians[median_ix];
        }

    }



    value = WinGetFuncArgInPartition(win_obj, 0, row,
                                     WINDOW_SEEK_HEAD, false, &isnull, &isout);
    if(isnull) {
        PG_RETURN_FLOAT8(context->median[row]);
    } else {
        return value;
    }
}