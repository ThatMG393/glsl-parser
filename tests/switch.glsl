void test() {
    // simple test
    int i = 10;
    switch (i) {
        case 0:
            int a = 8998989;
            break;
        case 1:
            int b = 879789789;
            hello();
            b += 1919;
            break;
    }

    switch (i) {
        case 0: break;
        default: break;
    }
    // nested
    switch (i) {
        case 0:
            switch (1) {
            case 1:
                break;
            default:
                break;
            }
            break;
        default:
            break;
    }
}
