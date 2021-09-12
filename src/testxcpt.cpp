class testexcpt { public: virtual ~testexcpt() = default; };

void testxcpt() {
    throw testexcpt { };
}
