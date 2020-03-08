(define accumulator 0)

(define accumulator2 0)

(define sum 0)

(while (not (= accumulator 6))
  (begin
    (set! accumulator2 0)
    (while (not (= accumulator2 7))
      (begin
        (set! sum (add1 sum))
        (set! accumulator2 (add1 accumulator2))))
    (set! accumulator (add1 accumulator))))