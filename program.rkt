(set! a (* (+ ALIGN (& d)))) (; a = *(&d + 1))

(set! c (- 0 a))

(set! d (add1 d))

(define a 0)

(define c 0)

(define d 4)

(define d2 5)

(define ALIGN 4)
