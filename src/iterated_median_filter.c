#include <math.h>
#include "postgres.h"
#include "fmgr.h"
#include "windowapi.h"
#include "quick_select.h"

PG_MODULE_MAGIC;


typedef struct {
    char is_error;
    double filter[1];
} iterated_median_filter_context;


PG_FUNCTION_INFO_V1(iterated_median_filter);

Datum iterated_median_filter(PG_FUNCTION_ARGS) {
    WindowObject win_obj = PG_WINDOW_OBJECT();
    uint32_t row = WinGetCurrentPosition(win_obj);
    uint32_t nelems = WinGetPartitionRowCount(win_obj);
    iterated_median_filter_context *context = WinGetPartitionLocalMemory(win_obj,
                                                                         sizeof(iterated_median_filter_context) +
                                                                         nelems * sizeof(double));

    if (row == 0) /* beginning of the partition; do all of the work now */
    {
        uint32_t i;
        uint32_t j;
        double *vals_last;
        double *med;
        Datum value;
        bool isnull, isout;

        bool window_size_is_null;
        Datum window_size_datum = WinGetFuncArgCurrent(win_obj, 1, &window_size_is_null);
        uint32_t window_size = DatumGetInt32(window_size_datum);

        bool epsilon_is_null;
        Datum epsilon_datum = WinGetFuncArgCurrent(win_obj, 2, &epsilon_is_null);
        double epsilon = DatumGetFloat8(epsilon_datum);

        context->is_error = 1; /* until proven otherwise */


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

        /* Validate input parameters */
        if (epsilon_is_null || !(epsilon > 0)) {
            ereport(ERROR,
                    (
                            errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                                    errmsg("epsilon must be greater than 0."),
                                    errdetail("value %f 0, or less than 0.", epsilon),
                                    errhint("make it slightly larger.")
                    )
            );
            PG_RETURN_NULL();
        }

        vals_last = palloc(nelems * sizeof(double));
        med = palloc(window_size * sizeof(double));
        /* Load the partition into the array */
        for (i = 0; i < nelems; i++) {
            value = WinGetFuncArgInPartition(win_obj, 0, i,
                                             WINDOW_SEEK_HEAD, false, &isnull, &isout);
            if (!isnull && !isout) {
                context->filter[i] = DatumGetFloat8(value);
            }
        }


        uint32_t offset = (window_size - 1) / 2;
        double median;
        uint32_t changed;
        do {
            changed = 0;
            /*elog(INFO, "FILTER PASS");	*/
            memcpy(vals_last, context->filter, nelems * sizeof(double));
            for (i = 0; i < nelems; i++) {
                memset(med, 0.0, window_size * sizeof(double));
                for (j = 0; j < window_size; j++) {
                    if (((i + j - offset) < nelems) && ((i + j) >= offset)) {
                        med[j] = vals_last[i + j - offset];
                    }
                }

                median = quick_select(med, window_size);
                context->filter[i] = median;
                changed = changed || (fabs(median - vals_last[i]) > epsilon);
                /*elog(INFO, "%f, %f, %f, %d", median, vals_last[i], fabs(median-vals_last[i]), changed);	*/
            }
        } while (changed != 0); /* Iterate until converged */


        pfree(vals_last);
        pfree(med);
    }


    PG_RETURN_FLOAT8(context->filter[row]);
}