
(if true
    (print "true!")
    (print "false!"))

(if false
    (print "false!")
    (print "true!"))

(if (eq 3 3)
    (print "3=3")
    (print "3!=3"))

(if (eq (+ 1 1) 2)
    (print "1 + 1 = 2"))

(if (> 4 3)
    (print "4 > 3"))

(if (< 3 4)
    (print "3 < 4"))

(if (<= 3 3)
    (print "3 <= 3"))

(if (>= 3 3)
    (print "3 >= 3"))

(print
    (if false true false))
