;; helper.scm - utilities
;; $Id$

;; Do not use them for serious jobs.  They are not
;; efficient.

;; HTML supports
;; (escape-html string) - escape dangerous characters
(define (escape-html str)
  (let loop ((lst (string->list str)) (done '()))
    (if (null? lst) (list->string (reverse done))
	(let ((c (car lst)))
	  (if (eq? c #\<) (loop (cdr lst) (append '(#\; #\t #\l #\&) done))
	  (if (eq? c #\>) (loop (cdr lst) (append '(#\; #\t #\g #\&) done))
	  (if (eq? c #\") (loop (cdr lst) (append '(#\; #\t #\o #\u #\q #\&) done))
	  (if (eq? c #\&) (loop (cdr lst) (append '(#\; #\p #\m #\a #\&) done))
	      (loop (cdr lst) (cons c done))))))))))

(define (escape-pod str)
  (let loop ((lst (string->list str)) (done '()))
    (if (null? lst) (list->string (reverse done))
	(let ((c (car lst)))
	  (if (eq? c #\<) (loop (cdr lst) (append '(#\> #\t #\l #\< #\E) done))
	  (if (eq? c #\>) (loop (cdr lst) (append '(#\> #\t #\g #\< #\E) done))
	  (if (eq? c #\") (loop (cdr lst) (append '(#\> #\t #\o #\u #\q #\< #\E) done))
	  (if (eq? c #\&) (loop (cdr lst) (append '(#\> #\p #\m #\a #\< #\E) done))
	      (loop (cdr lst) (cons c done))))))))))


;; (link-mail email) - make a link to the email address
(define (link-mail address)
  (string-append "&lt;<a href=\"mailto:" address "\">" address "</a>&gt;"))

;; (footer) - print out a footer
(define (footer)
  (display "<hr /><div>This document was converted ")
  (if *escm-cgi-script*
      (display "on-line") (display "off-line"))
  (display " by ") (display PACKAGE) (display " (version ")
  (display VERSION) (display ") on ") (display  date)
  (display ".</div>\n"))

;; (header up prev next) - print out a header
(define (header up prev next)
  (if (or up prev next)
      (begin
	(display "<div class=\"header\">")
	(if up
	    (begin
	      (display "[<a href=\"") (display up)
	      (display "\">Up</a>]\n")))
	(if prev
	    (begin
	      (display "[<a href=\"") (display prev)
	      (display "\">Prev</a>]\n")))
	(if next
	    (begin
	      (display "[<a href=\"") (display next)
	      (display "\">Next</a>]\n")))
	(display "</div><hr />\n"))))

;; Cases
(define (string-upcase str)
  (let loop ((lst (string->list str)) (done '()))
    (if (null? lst) (list->string (reverse done))
	(loop (cdr lst) (cons (char-upcase (car lst)) done)))))

(define (string-downcase str)
  (let loop ((lst (string->list str)) (done '()))
    (if (null? lst) (list->string (reverse done))
	(loop (cdr lst) (cons (char-downcase (car lst)) done)))))

;; this implementation is not correct.
(define (string-titlecase str)
  (let ((lst (string->list str)))
    (list->string
     (cons (char-upcase (car lst))
	   (let loop ((lst (cdr lst)) (done '()))
	     (if (null? lst) (reverse done)
		 (loop (cdr lst) (cons (char-downcase (car lst)) done))))))))
;;; end of helper.scm
