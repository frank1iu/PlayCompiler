(define __COMPILER_ALIGN 4)

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

(define __COMPILER_HEAP_BP 65024) (; 0xfe00)
(define (malloc size)
    (begin (define! temp __COMPILER_HEAP_BP)
        (set! __COMPILER_HEAP_BP (+ __COMPILER_HEAP_BP size))
        temp))

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
        (call mult n (call factorial (sub1 n)))))
(void (call expect (call factorial 3) 6))
(void (call expect (call factorial 4) 24))
(void (call expect (call factorial 5) 120))