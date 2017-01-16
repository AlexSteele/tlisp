
(def fout (open "something.txt" "w"))
(print (write fout "HELLO WORLD\n"))
(close fout)
(print "DONE")

(def fin (open "something.txt" "r"))
(print (read-line fin))
(close fin)
(print "DONE")

(def fin2 (open "IDONOTEXIST" "r"))
(print (eq fin2 nil))
