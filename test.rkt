(set! c (& d))          (; c = &d)

(set! c (+ c ALIGN))    (; c = (d + 1))

(set! c (* c))          (; c = *(d + 1) = e)

(define c 0)

(define d 4)

(define d2 5)

(define ALIGN 4)