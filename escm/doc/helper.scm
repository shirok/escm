;; helper.scm
;; $Id$
;; Author: TAGA Yoshitaka

;; Do not use them for serious jobs.  They are not
;; efficient.

(define (header)
  (display "<p><em>")
  (display *escm-output-file*)
  (display "</em> is part of documentation of <strong>")
  (display PACKAGE) (display " ") (display VERSION)
  (display "</strong>. This file was converted from <em>")
  (display *escm-input-file*)
  (display "</em> ")
  (display " on ")(display *date*)(display ".\n")
  (display "The backend is <em>")
  (display *escm-interpreter*)
  (display "</em>.</p>\n"))

(define (escape-html str)
  (let loop ((lst (string->list str)) (done '()))
    (if (null? lst) (list->string (reverse done))
	(let ((c (car lst)))
	  (if (eq? c #\<) (loop (cdr lst) (append '(#\; #\t #\l #\&) done))
	  (if (eq? c #\>) (loop (cdr lst) (append '(#\; #\t #\g #\&) done))
	  (if (eq? c #\") (loop (cdr lst) (append '(#\; #\t #\o #\u #\q #\&) done))
	  (if (eq? c #\&) (loop (cdr lst) (append '(#\; #\p #\m #\a #\&) done))
	      (loop (cdr lst) (cons c done))))))))))

(define (mail-link addr)
  (string-append "<a href=\"mailto:" addr "\">" addr "</a>"))

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


;(define (sourceforge-logo)
;  "<a href=\"http://sourceforge.net\"><img src=\"http://sourceforge.net/sflogo.php?group_id=68657&type=1\" width=\"88\" height=\"31\" border=\"0\" alt=\"SourceForge.net Logo\" /></a>")

;;; end of helper.scm
