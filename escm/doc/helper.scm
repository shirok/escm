;; helper.scm
;; $Id$
;; Author: TAGA Yoshitaka

;; Do not use them for serious jobs.  They are not
;; efficient.

;;; SourceForge Logo
(define *sf-group-id* "68657")
(define *sf-logo-data* '("1" "88" "31"))
(define (%sf-logo-type) (car *sf-logo-data*))
(define (%sf-logo-width) (cadr *sf-logo-data*))
(define (%sf-logo-height) (caddr *sf-logo-data*))
(define (sourceforge-logo)
  (string-append
   "<a href=\"http://sourceforge.net\">"
   "<img src=\"http://sourceforge.net/sflogo.php?group_id="
   *sf-group-id* "&amp;type=" (%sf-logo-type)
   "\" width=\"" (%sf-logo-width) "\" height=\"" (%sf-logo-height)
   "\" alt=\"SourceForge.net Logo\" /></a>"))

;;; HTML
;; (escape-html str) -  escape characters used in HTML tags.
(define (escape-html str)
  (let loop ((lst (string->list str)) (done '()))
    (if (null? lst) (list->string (reverse done))
	(let ((c (car lst)))
	  (if (eq? c #\<) (loop (cdr lst) (append '(#\; #\t #\l #\&) done))
	  (if (eq? c #\>) (loop (cdr lst) (append '(#\; #\t #\g #\&) done))
	  (if (eq? c #\") (loop (cdr lst) (append '(#\; #\t #\o #\u #\q #\&) done))
	  (if (eq? c #\&) (loop (cdr lst) (append '(#\; #\p #\m #\a #\&) done))
	      (loop (cdr lst) (cons c done))))))))))

;; (link-mail addr) - link to an email address.
(define (link-mail addr)
  (string-append "<a href=\"mailto:" addr "\">" addr "</a>"))
;; (link-url addr) - link to a URL.
(define (link-url url)
  (string-append "<a href=\"" url "\">" url "</a>"))

;;; Cases
(define (string-upcase str)
  (let loop ((lst (string->list str)) (done '()))
    (if (null? lst) (list->string (reverse done))
	(loop (cdr lst) (cons (char-upcase (car lst)) done)))))

(define (string-downcase str)
  (let loop ((lst (string->list str)) (done '()))
    (if (null? lst) (list->string (reverse done))
	(loop (cdr lst) (cons (char-downcase (car lst)) done)))))
; this implementation is not correct.
(define (string-titlecase str)
  (let ((lst (string->list str)))
    (list->string
     (cons (char-upcase (car lst))
	   (let loop ((lst (cdr lst)) (done '()))
	     (if (null? lst) (reverse done)
		 (loop (cdr lst) (cons (char-downcase (car lst)) done))))))))

;;; URL
(define %hexdec
  '((#\0 . 0) (#\1 . 1) (#\2 . 2) (#\3 . 3) (#\4 . 4)
    (#\5 . 5) (#\6 . 6) (#\7 . 7) (#\8 . 8) (#\9 . 9)
    (#\A . 10) (#\B . 11) (#\C . 12) (#\D . 13) (#\E . 14) (#\F . 15)))
(define (%char->hex-rlist c)
  (let* ((code (char->integer c))
	 (upper (quotient code 16))
	 (lower (remainder code 16)))
    (list (car (list-ref %hexdec lower)) (car (list-ref %hexdec upper)) #\%)))
;; encode-url
(define (encode-url url)
  (let loop ((from (string->list url)) (to '()))
    (if (null? from) (list->string (reverse to))
	(let ((c (car from)))
	  (if (or (char-alphabetic? c) (char-numeric? c))
	      (loop (cdr from) (cons c to))
	      (loop (cdr from) (append (%char->hex-rlist c) to)))))))
;; decode-url
(define (decode-url url)
  (let loop ((from (string->list url)) (to '()))
    (if (null? from) (list->string (reverse to))
	(let ((c (car from)))
	  (if (eq? c #\+)
	      (loop (cdr from) (cons #\space to))
	  (if (eq? c #\%)
	      (let ((upper (cdr (assq (cadr from) %hexdec)))
		    (lower (cdr (assq (caddr from) %hexdec))))
		(loop (cdddr from)
		      (cons (integer->char (+ (* 16 upper) lower)) to)))
	      (loop (cdr from) (cons c to))))))))

;; decompose-query
(define (%mk-var var) (list->string (reverse var)))
(define (%mk-val val) (decode-url (list->string (reverse val))))
(define (decompose-query str)
  (let loop ((from (string->list str)) (to '()))
    (if (null? from) to
	(let var-loop ((from from) (var '()))
	  (if (null? from)
	      (cons (cons (%mk-var var) "") to)
	      (let ((c (car from)))
		(if (char=? c #\&)
		    (loop (cdr from)
			  (cons (cons (%mk-var var) "") to))
		(if (char=? c #\=)
		    (let val-loop ((from (cdr from)) (val '()))
		      (if (null? from)
			  (cons (cons (%mk-var var) (%mk-val val)) to)
			  (let ((c (car from)))
			    (if (char=? c #\&)
				(loop (cdr from)
				      (cons (cons (%mk-var var) (%mk-val val))
					    to))
				(val-loop (cdr from) (cons c val))))))
		    (var-loop (cdr from) (cons c var))))))))))
;;; end of helper.scm
