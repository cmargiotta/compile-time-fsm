#!/usr/bin/env guile
!#

(use-modules (ice-9 rdelim)
             (ice-9 format))

(define (concat-files input-files output-file)
  (call-with-output-file output-file
    (λ (output-port)
      (for-each
       (λ (input-file)
         (call-with-input-file input-file
           (λ (input-port)
             (let loop ()
               (let ((line (read-line input-port)))
                 (unless (eof-object? line)
                   (display line output-port)
                   (newline output-port)
                   (loop)))))))
       input-files))))

(define (main args)
  (if (< (length args) 3)
      (format #t "Usage: ~a file_output file1 file2 ... fileN~%" (car args))
      (let ((output-file (cadr args))
	    (input-files (cddr args)))
        (concat-files input-files output-file)
        (format #t "File ~a ready%"
                output-file))))

(main (command-line))
