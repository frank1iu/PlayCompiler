(set! c (& d2))          (; c = &d2)

(set! c (- c ALIGN))    (; c = d2 - 1)

(set! c (* c))          (; c = *(d - 1) = d)

(set! c (add1 c))       (; c += 1)

(define c 0)

(define d 4)

(define d2 5)

(define ALIGN 4)