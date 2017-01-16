
(def name "matrix")
(def secret-num 5)
(def say-hi
  (lambda (name n)
          (if (< n 5)
            (print "HI" name)
            (print "BYE" name))))

(say-hi name secret-num)

(def add (lambda (x y) (+ x y)))
(print (add 3 secret-num))
