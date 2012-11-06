#include "RenderScript.h"
#include <sys/time.h>

#include "ScriptC_latency.h"

using namespace android;
using namespace renderscriptCpp;

int main(int argc, char** argv)
{
    int iters = 100;

    int numElems = 1000;

    if (argc >= 2) {
        iters = atoi(argv[1]);
        if (iters <= 0) {
            printf("iters must be positive\n");
            return 1;
        }
    }

    printf("iters = %d\n", iters);

    if (argc >= 3) {
        numElems = atoi(argv[2]);
        if (numElems <= 0) {
            printf("numElems must be positive\n");
            return 1;
        }
    }

    printf("numElems = %d\n", numElems);

    sp<RS> rs = new RS();

    bool r = rs->init(true); // force CPU execution

    sp<const Element> e = Element::U32(rs);

    Type::Builder tb(rs, e);
    tb.setX(numElems);
    sp<const Type> t = tb.create();

    sp<Allocation> ain = Allocation::createTyped(rs, t);
    sp<Allocation> aout = Allocation::createTyped(rs, t);

    sp<ScriptC_latency> sc = new ScriptC_latency(rs, NULL, 0);

    struct timeval start, stop;

    gettimeofday(&start, NULL);

    for (int i = 0; i < iters; i++) {
        sc->forEach_root(ain, aout);
    }

    uint32_t temp;

    aout->copy1DRangeFromUnchecked(0, 1, &temp, sizeof(temp));

    gettimeofday(&stop, NULL);

    long long elapsed = (stop.tv_sec * 1000000) - (start.tv_sec * 1000000) + (stop.tv_usec - start.tv_usec);
    printf("elapsed time : %lld microseconds\n", elapsed);
    printf("time per iter: %f microseconds\n", (double)elapsed / iters);

    sc.clear();
    t.clear();
    e.clear();
    ain.clear();
    aout.clear();
}
