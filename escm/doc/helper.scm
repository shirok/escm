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
   "\" border=\"0\""
   " alt=\"SourceForge.net Logo\" /></a>"))

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
(define (link-url url . anchor)
  (if anchor
    (string-append "<a href=\"" url "\">" (car anchor) "</a>")
    (string-append "<a href=\"" url "\">" url "</a>")))

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
;;; end of helper.scm
