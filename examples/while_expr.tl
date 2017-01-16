
(def x 0)
(while (not (eq x 15))
  (do
    (print x)
    (set! x (+ x 1))))

(print "toodloo!")
