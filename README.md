# PlayCompiler: Racket to SM213 Assembly

## Requirements

- Any C++ compiler
- Node.js

## Building

```sh
make
```

## Usage

```sh
./compile.sh program.rkt
```
The output will be stored in program.rkt.s

## Example

### Code

```lisp
;; a program that creates an array and populates it so that each
;; value is equal to its index, if the index is greater than 4

;; int -> #true or #false
;; produces true if num is bigger than 4
(define (is_big num)
    (> num 4))
;; tests for is_big
(void (call (expect (is_big 0) #false)))
(void (call (expect (is_big 4) #false)))
(void (call (expect (is_big 5)  #true)))

(define array_start #nullptr) ;; int*

;; preprocessor constants are defined like this
;#define #arr_size 10

(set! array_start (call array_new #arr_size))

(for                            ;; C equivalent:
    (define! index 0)           ;; int index = 0
    (< index #arr_size)         ;; index < arr_size
    (set! index (add1 index))   ;; index++
    (if (call is_big index)
        (call array_set array_start index index)
        (call array_set array_start index 0)))
```

### Preprocessor Output
```lisp
(define (is_big num) 
(> num 5)) 
(define array_start 0) 
(set! array_start (call array_new 10)) 
(for 
(define! index 0) 
(< index 10) 
(set! index (add1 index)) 
(if (call is_big index) 
(call array_set array_start index index) 
(call array_set array_start index 0)))
```

### Compiler Output (Abridged - stdlib functions omitted)
```asm
ld  $stacktop, r5
nop		# [begin function call: array_new]
ld  $-4, r0		# [stack allocation for num]
add r0, r5		# [stack allocation for num]
ld  $10, r0		# 10
st  r0, (r5)		# (define! num 10)
gpc $6, r6		# (call array_new 10)
j   array_new		# (call array_new 10)
ld  $8, r0		# [stack shrink]
add r0, r5		# [stack shrink]
ld  $0, r0		# [stack shrink]
add r0, r5		# [stack shrink]
mov r7, r0		# (call array_new 10)
ld  $array_start, r1		# (set! array_start (call array_new 10))
st  r0, (r1)		# (set! array_start (call array_new 10))
ld  $-4, r0		# [stack allocation for index]
add r0, r5		# [stack allocation for index]
ld  $0, r0		# 0
st  r0, (r5)		# (define! index 0)
	L52:		# (while (< index 10) (begin (if (call is_big index) (call array_set array_start index index) (call array_set array_start index 0)) (set! index (add1 index))))
ld  $10, r0		# 10
ld  0(r5), r1		# index
not r1		# (not index)
inc r1		# (add1 (not index))
add r0, r1		# (+ 10 (add1 (not index)))
bgt r1, L55		# (< index 10)
ld  $0, r0		# (< index 10)
br  L56		# (< index 10)
	L55:		# (< index 10)
ld  $1, r0		# (< index 10)
	L56:		# (< index 10)
bgt r0, L53		# (while (< index 10) (begin (if (call is_big index) (call array_set array_start index index) (call array_set array_start index 0)) (set! index (add1 index))))
j   L54		# (while (< index 10) (begin (if (call is_big index) (call array_set array_start index index) (call array_set array_start index 0)) (set! index (add1 index))))
	L53:		# (while (< index 10) (begin (if (call is_big index) (call array_set array_start index index) (call array_set array_start index 0)) (set! index (add1 index))))
			 # Frame created. ID: 80		# (begin (if (call is_big index) (call array_set array_start index index) (call array_set array_start index 0)) (set! index (add1 index)))
nop		# [begin function call: is_big]
ld  $-4, r0		# [stack allocation for num]
add r0, r5		# [stack allocation for num]
ld  4(r5), r0		# index
st  r0, (r5)		# (define! num index)
gpc $6, r6		# (call is_big index)
j   is_big		# (call is_big index)
ld  $8, r0		# [stack shrink]
add r0, r5		# [stack shrink]
ld  $0, r0		# [stack shrink]
add r0, r5		# [stack shrink]
mov r7, r0		# (call is_big index)
bgt r0, L57		# (if (call is_big index) (call array_set array_start index index) (call array_set array_start index 0))
nop		# [begin function call: array_set]
ld  $-4, r0		# [stack allocation for base2]
add r0, r5		# [stack allocation for base2]
ld  $array_start, r0		# array_start
ld  (r0), r0		# array_start
st  r0, (r5)		# (define! base2 array_start)
ld  $-4, r0		# [stack allocation for index]
add r0, r5		# [stack allocation for index]
ld  8(r5), r0		# index
st  r0, (r5)		# (define! index index)
ld  $-4, r0		# [stack allocation for val]
add r0, r5		# [stack allocation for val]
ld  $0, r0		# 0
st  r0, (r5)		# (define! val 0)
gpc $6, r6		# (call array_set array_start index 0)
j   array_set		# (call array_set array_start index 0)
ld  $16, r0		# [stack shrink]
add r0, r5		# [stack shrink]
ld  $0, r0		# [stack shrink]
add r0, r5		# [stack shrink]
mov r7, r0		# (call array_set array_start index 0)
br  L58		# (if (call is_big index) (call array_set array_start index index) (call array_set array_start index 0))
	L57:		# (if (call is_big index) (call array_set array_start index index) (call array_set array_start index 0))
ld  $-4, r0		# [stack allocation for r0]
add r0, r5		# [stack allocation for r0]
st  r0, (r5)		# [function call: save registers] [call to array_set]
nop		# [begin function call: array_set]
ld  $-4, r0		# [stack allocation for base2]
add r0, r5		# [stack allocation for base2]
ld  $array_start, r0		# array_start
ld  (r0), r0		# array_start
st  r0, (r5)		# (define! base2 array_start)
ld  $-4, r0		# [stack allocation for index]
add r0, r5		# [stack allocation for index]
ld  12(r5), r0		# index
st  r0, (r5)		# (define! index index)
ld  $-4, r0		# [stack allocation for val]
add r0, r5		# [stack allocation for val]
ld  16(r5), r0		# index
st  r0, (r5)		# (define! val index)
gpc $6, r6		# (call array_set array_start index index)
j   array_set		# (call array_set array_start index index)
ld  $16, r0		# [stack shrink]
add r0, r5		# [stack shrink]
ld  0(r5), r0		# [function call: load saved registers] [call to array_set]
ld  $4, r0		# [stack shrink]
add r0, r5		# [stack shrink]
mov r7, r0		# (call array_set array_start index index)
	L58:		# (if (call is_big index) (call array_set array_start index index) (call array_set array_start index 0))
ld  0(r5), r0		# index
inc r0		# (add1 index)
st  r0, 0(r5)		# (set! index (add1 index))
			 # Frame info at time of destruction: 		# 
			 # ID: 80		# 
ld  $0, r0		# [stack shrink]
add r0, r5		# [stack shrink]
j   L52		# (while (< index 10) (begin (if (call is_big index) (call array_set array_start index index) (call array_set array_start index 0)) (set! index (add1 index))))
	L54:		# (while (< index 10) (begin (if (call is_big index) (call array_set array_start index index) (call array_set array_start index 0)) (set! index (add1 index))))
halt
.pos 0x4000		# Data Region
	is_big: 
ld  $-4, r0		# [stack allocation for __COMPILER_RETURN_ADDRESS]
add r0, r5		# [stack allocation for __COMPILER_RETURN_ADDRESS]
st  r6, (r5)		# (define! __COMPILER_RETURN_ADDRESS [r6])
ld  4(r5), r0		# num
ld  $5, r1		# 5
not r1		# (not 5)
inc r1		# (add1 (not 5))
add r0, r1		# (+ num (add1 (not 5)))
bgt r1, L50		# (> num 5)
ld  $0, r0		# (> num 5)
br  L51		# (> num 5)
	L50:		# (> num 5)
ld  $1, r0		# (> num 5)
	L51:		# (> num 5)
mov r0, r7 		# [copying return value to r7 for is_big]
ld  0(r5), r6 		# [load return address for is_big]
j   (r6) 		# [function return for is_big]
```