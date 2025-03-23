void test() {
    do a(); while (true);
    do { } while (true);

	int i = 0;
    do {
    	if ((i % 0) == 0) {
    		i--;
    	}
    	a();
    	b();
    } while (i < 69);
}
