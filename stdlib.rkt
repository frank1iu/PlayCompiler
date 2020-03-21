(begin
(define __COMPILER_DEBUG_VAL0 0)
(define __COMPILER_DEBUG_VAL1 0)
(define __COMPILER_DEBUG_VAL2 0)
(define __COMPILER_DEBUG_VAL3 0)
(define __COMPILER_DEBUG_VAL4 0)
(define __COMPILER_DEBUG_VAL5 0)
(define __COMPILER_DEBUG_VAL6 0)
(define __COMPILER_DEBUG_VAL7 0)
(define __COMPILER_DEBUG_VAL8 0))

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
(void (call expect (call mult 5 5) 25))
(void (call expect (call mult 5 6) 30))
(void (call expect (call mult -2 7) -14))

(define (leq x y)
    (if (> x y)
    #false
    #true))
(void (call expect (call leq 5 5) #true))
(void (call expect (call leq 4 5) #true))
(void (call expect (call leq -5 5) #true))
(void (call expect (call leq 6 5) #false))

(define (geq x y)
    (if (< x y)
    #false
    #true))
(void (call expect (call geq 5 5) #true))
(void (call expect (call geq 4 5) #false))
(void (call expect (call geq -5 5) #false))
(void (call expect (call geq 6 5) #true))

(define (remainder _big small)
    (begin
        (define! big _big)
        (while (call geq big small)
            (set! big (- big small)))
        big))
(void (call expect (call remainder 12 5) 2))
(void (call expect (call remainder 31 5) 1))
(void (call expect (call remainder 36 6) 0))

(define (factorial n)
    (if (call leq n 1)
        1
        (call mult (call factorial (sub1 n)) n)))
(void (call expect (call factorial 3) 6))
(void (call expect (call factorial 4) 24))
(void (call expect (call factorial 5) 120))

(define __COMPILER_ALIGN 4)
(define __COMPILER_HEAP_BP 65540)
(define (malloc size)
    (begin
        (write! __COMPILER_HEAP_BP size)
        (set! __COMPILER_HEAP_BP (+ __COMPILER_ALIGN __COMPILER_HEAP_BP))
        (define! temp __COMPILER_HEAP_BP)
        (set! __COMPILER_HEAP_BP (+ __COMPILER_HEAP_BP size))
        temp))

(define (array_new num)
    (begin
        (define! base (call malloc (call mult (add1 num) __COMPILER_ALIGN)))
        (write! base num)
        (+ base __COMPILER_ALIGN)))

(define (array_length base)
    (* (- base __COMPILER_ALIGN)))

(define (array_at base index)
    (if (call geq index (call array_length base))
        (call assert #false)
        (begin
            (* (+ (call mult index __COMPILER_ALIGN) base)))))

(define (array_set base index val)
    (if (call geq index (call array_length base))
        (call assert #false)
        (begin
            (write! (+ (call mult index __COMPILER_ALIGN) base) val))))

(define (array_empty base)
    (= (call array_length base) 0))