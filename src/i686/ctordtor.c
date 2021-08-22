typedef void (*ctordtor)(void);

extern ctordtor __ctors_start[];
extern ctordtor *__ctors_end;
extern ctordtor __dtors_start[];
extern ctordtor *__dtors_end;

void _ctors(void) {
    ctordtor *current = __ctors_start;
    for (; current != __ctors_end; ++current) {
        (*current)();
    }
}

void _dtors(void) {
    ctordtor *current = __dtors_start;
    for (; current != __dtors_end; ++current) {
        (*current)();
    }
}
