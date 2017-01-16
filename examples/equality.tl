
(print (eq 1 1))

(print (eq "hi" "hi"))

(def x '(1 2 3))
(print (eq x x))
(print (eq x '(1 2 3)))

(print (eq print print))

(def my-func (lambda (x) x))
(print (eq my-func my-func))
