;#define #true 1
;#define #false 0
;#define #nullptr 0
;#eval $.DEBUG = true

;#if $.DEBUG
(define __COMPILER_DEBUG_VAL0 0)
(define __COMPILER_DEBUG_VAL1 0)
(define __COMPILER_DEBUG_VAL2 0)
(define __COMPILER_DEBUG_VAL3 0)
(define __COMPILER_DEBUG_VAL4 0)
(define __COMPILER_DEBUG_VAL5 0)
(define __COMPILER_DEBUG_VAL6 0)
(define __COMPILER_DEBUG_VAL7 0)
(define __COMPILER_DEBUG_VAL8 0)
(define __COMPILER_DEBUG_VAL9 0)
;#endif

(define (assert cond)
    (if cond 0 (while #true 0)))

(define (expect val1 val2)
    (call assert (= val1 val2)))

(define (mult x y)
    (begin
        (define! temp x)
        (for
            (define! j 1)   (< j y)   (set! j (add1 j))
            (set! x (+ temp x)))
        x))

(define (leq x y)
    (if (> x y)
    #false
    #true))

(define (geq x y)
    (if (< x y)
    #false
    #true))

(define (remainder _big small)
    (begin
        (define! big _big)
        (while (call geq big small)
            (set! big (- big small)))
        big))

(define (factorial n)
    (if (call leq n 1)
        1
        (call mult (call factorial (sub1 n)) n)))

(define __COMPILER_ALIGN 4)
(define __COMPILER_HEAP_BP 65540)
(define (malloc size)
    (begin
        (write! __COMPILER_HEAP_BP -1)
        (set! __COMPILER_HEAP_BP (+ __COMPILER_ALIGN __COMPILER_HEAP_BP))
        (write! __COMPILER_HEAP_BP size)
        (set! __COMPILER_HEAP_BP (+ __COMPILER_ALIGN __COMPILER_HEAP_BP))
        (define! temp __COMPILER_HEAP_BP)
        (set! __COMPILER_HEAP_BP (+ __COMPILER_HEAP_BP size))
        temp))

(define (array_new num)
    (begin
        (define! base4 (call malloc (call mult (add1 num) __COMPILER_ALIGN)))
        (write! base4 num)
        (+ base4 __COMPILER_ALIGN)))

(define (array_length base0)
    (* (- base0 __COMPILER_ALIGN)))

(define (array_at base1 index)
    (if (call geq index (call array_length base1))
        (call assert #false)
        (begin
            (* (+ (call mult index __COMPILER_ALIGN) base1)))))

(define (array_set base2 index val)
    (if (call geq index (call array_length base2))
        (call assert #false)
        (begin
            (write! (+ (call mult index __COMPILER_ALIGN) base2) val))))

(define (array_empty base3)
    (= (call array_length base3) 0))

(define (array_rest base)
    (if (call array_empty base)
        (call assert #false)
        (begin
            (define! start (call array_new (sub1 (call array_length base))))
            (for (define! i 0) (< i (call array_length start)) (set! i (add1 i))
                (call array_set start i (call array_at base (add1 i))))
            start)))

(define (array_first base)
        (call array_at base 0))