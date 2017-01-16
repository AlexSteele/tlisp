
(def if-expand (macro (test if-true if-false)
  `(if ~test ~if-true ~if-false)))

(if-expand (> 3 4) (print "YES") (print "NO"))

(def for (macro (binding in list action)
  `(let (xs ~list)
     (while (not (eq xs nil))
       (let (~binding (car xs))
         (do
             ~action
             (set! xs (cdr xs))))))))

(def things '(1 2 3 4 5))
(for thing in things
     (print thing))
