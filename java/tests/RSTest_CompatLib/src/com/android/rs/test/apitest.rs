#include "shared.rsh"

rs_element elemNull;
rs_element elemNonNull;
rs_type typeNull;
rs_type typeNonNull;
rs_allocation allocNull;
rs_allocation allocNonNull;
rs_sampler samplerNull;
rs_sampler samplerNonNull;
rs_script scriptNull;
rs_script scriptNonNull;


static bool test_obj_api() {
    bool failed = false;

    _RS_ASSERT(!rsIsObject(elemNull));
    _RS_ASSERT(rsIsObject(elemNonNull));
    rsSetObject(&elemNull, elemNonNull);
    _RS_ASSERT(rsIsObject(elemNull));
    rsClearObject(&elemNull);
    _RS_ASSERT(!rsIsObject(elemNull));

    _RS_ASSERT(!rsIsObject(typeNull));
    _RS_ASSERT(rsIsObject(typeNonNull));
    rsSetObject(&typeNull, typeNonNull);
    _RS_ASSERT(rsIsObject(typeNull));
    rsClearObject(&typeNull);
    _RS_ASSERT(!rsIsObject(typeNull));

    _RS_ASSERT(!rsIsObject(allocNull));
    _RS_ASSERT(rsIsObject(allocNonNull));
    rsSetObject(&allocNull, allocNonNull);
    _RS_ASSERT(rsIsObject(allocNull));
    rsClearObject(&allocNull);
    _RS_ASSERT(!rsIsObject(allocNull));

    _RS_ASSERT(!rsIsObject(samplerNull));
    _RS_ASSERT(rsIsObject(samplerNonNull));
    rsSetObject(&samplerNull, samplerNonNull);
    _RS_ASSERT(rsIsObject(samplerNull));
    rsClearObject(&samplerNull);
    _RS_ASSERT(!rsIsObject(samplerNull));

    _RS_ASSERT(!rsIsObject(scriptNull));
    _RS_ASSERT(rsIsObject(scriptNonNull));
    rsSetObject(&scriptNull, scriptNonNull);
    _RS_ASSERT(rsIsObject(scriptNull));
    rsClearObject(&scriptNull);
    _RS_ASSERT(!rsIsObject(scriptNull));

    if (failed) {
        rsDebug("test_obj_api FAILED", -1);
    }
    else {
        rsDebug("test_obj_api PASSED", 0);
    }

    return failed;
}

void api_test() {
    bool failed = false;
    failed |= test_obj_api();

    if (failed) {
        rsSendToClientBlocking(RS_MSG_TEST_FAILED);
    }
    else {
        rsSendToClientBlocking(RS_MSG_TEST_PASSED);
    }
}

