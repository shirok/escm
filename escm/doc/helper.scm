;; helper.scm
;; $Id$
;; Author: TAGA Yoshitaka

;; Do not use them for serious jobs.  They are not
;; efficient.

(use srfi-13) ;; string manipulations

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
;; (link-mail addr) - link to an email address.
(define (link-mail addr)
  (string-append "<a href=\"mailto:" addr "\">" addr "</a>"))
;; (link-url addr) - link to a URL.
(define (link-url url . anchor)
  (if (not (null? anchor))
    (string-append "<a href=\"" url "\">" (car anchor) "</a>")
    (string-append "<a href=\"" url "\">" url "</a>")))

;;; Footnote
(define *fn-cnt* 0)
(define *fn-lst* '())
(define (footnote str)
  (set! *fn-cnt* (+ 1 *fn-cnt*))
  (print "<sup>(<span id=\"fnm" *fn-cnt* "\">"
	 "<a href=\"#fnt" *fn-cnt* "\">" *fn-cnt* "</a></span>)</sup>"))

(define (footnote-list)
  (print "<dl>")
  (let loop ((fn 1) (lst (reverse *fn-lst)))
    (if (null? lst) #t
	(begin
	  (print "<dd id=\"fnt" fn "\">(<a href=\"#fnm" fn "\">" fn "</a>)</dd>")
	  (print "<dt><p>" (car lst) "</p></dt>")
	  (loop (+ fn 1) (cdr lst)))))
  (print "</dl>"))

;;; Useful Links
(define escm
  (link-url "http://www.shiro.dreamhost.com/scheme/vault/escm.html"
	    "escm 1.1"))
(define gauche
  (link-url "http://sourceforge.net/projects/gauche/"
	    "Gauche"))
(define guile
  (link-url "http://www.gnu.org/software/guile/guile.html"
	    "Guile"))
(define scm
  (link-url "http://swissnet.ai.mit.edu/~jaffer/SCM.html"
               "SCM"))



;;; end of helper.scm
