;; -*- coding: utf-8 -*-
;;
;; gen_eaw_cdata.scm
;; 2020-12-8 v1.03
;;
;; ＜内容＞
;;   Gauche を使用して、C言語用の文字幅データを生成するためのツールです。
;;   EastAsianWidth.txt と emoji-data.txt が、本ファイルと同一フォルダに
;;   存在することを想定しています。
;;   EastAsianWidth.txt と emoji-data.txt は、以下にあります。
;;     https://unicode.org/Public/UNIDATA/EastAsianWidth.txt
;;     https://unicode.org/Public/UNIDATA/emoji/emoji-data.txt
;;
;; ＜使い方＞
;;   gosh gen_eaw_cdata.scm mode
;;     mode  動作モード
;;       =0:テスト用の文字幅データ(wide-test.txt)を生成する
;;       =1:Full Width 用の文字幅データ(wide-full.txt)を生成する
;;       =2:Ambiguous Width 用の文字幅データ(wide-ambiguous.txt)を生成する
;;       =3:Emoji 用の文字幅データ(wide-emoji.txt)を生成する(実験中)
;;
(use util.match)

;; EastAsianWidth ファイル (#f なら読み込まない)
(define east-asian-width-file "EastAsianWidth.txt")

;; emoji-data ファイル     (#f なら読み込まない)
(define emoji-data-file       "emoji-data.txt")

;; 結果データ ファイル     (#f なら書き出さない)
(define result-data-file      "wide-data.txt")

;; ワイド文字セレクタ設定  (EastAsianWidth の A F H N Na W から選択)
(define wide-type-selector    '(F W))

;; ワイド文字範囲リストの追加設定 (#f なら追加しない)
;;   (UTF-16 でサロゲートペアになる文字は、すべてワイド文字と判定しないと、
;;    Windows コンソールの1行に収まらない (1文字で2セル使用するため))
(define wide-range-list-plus  '((#x10000 . #xeffff)))

;; 結果データのインデント設定
(define result-data-indent    8)


;; 範囲の構造体 (実体はコンスセル)
;;   start  範囲の開始点(整数)
;;   end    範囲の終了点(整数)
(define (make-range start end) (cons start end))
(define (copy-range r) (list-copy r))
(define range-start (getter-with-setter (lambda (r)   (car r))
                                        (lambda (r v) (set-car! r v))))
(define range-end   (getter-with-setter (lambda (r)   (cdr r))
                                        (lambda (r v) (set-cdr! r v))))

;; 範囲のリストのソート
;;   ・例. ((3 . 4) (1 . 2)) → ((1 . 2) (3 . 4))
;;   ・開始点でソートする
(define (sort-range-list range-list)
  (sort range-list < range-start))

;; 範囲のリストの圧縮
;;   ・例. ((100 . 110) (111 . 120)) → ((100 . 120))
;;   ・範囲のリストは、開始点でソート済みであること
(define (compress-range-list range-list)
  (define result-range-list '())
  (for-each
   (lambda (r-now)
     ;; 条件を満たせば、最後の範囲にマージする
     (let ((r-last (if (null? result-range-list)
                     #f
                     (car result-range-list))))
       (if (and r-last
                (<= (range-start r-now)  (+ (range-end r-last) 1))
                (<= (range-start r-last) (+ (range-end r-now)  1)))
         (set-car! result-range-list
                   (make-range (min (range-start r-now) (range-start r-last))
                               (max (range-end   r-now) (range-end   r-last))))
         (push! result-range-list r-now))))
   range-list)
  (reverse result-range-list))

;; 範囲のリストのマージ
;;   ・例. ((100 . 110) (120 . 130) (140 . 150)) + ((130 . 140))
;;         → ((100 . 110) (120 . 150))
;;   ・範囲のリスト1と2は、開始点でソート済みであること
(define (merge-range-list range-list-1 range-list-2)
  (define result-range-list '())
  (cond
   ((null? range-list-1) range-list-2)
   ((null? range-list-2) range-list-1)
   (else
    (let loop ((r-now #f)
               (r1    (car range-list-1))
               (r2    (car range-list-2))
               (rest1 range-list-1)
               (rest2 range-list-2))
      ;; r1 と r2 のうち開始点の小さい方を選択
      (cond
       ((and r1
             (or (not r2)
                 (<= (range-start r1) (range-start r2))))
        (set! r-now r1)
        (set! rest1 (cdr rest1)))
       (else
        (set! r-now r2)
        (set! rest2 (cdr rest2))))
      ;; 条件を満たせば、最後の範囲にマージする
      (let ((r-last (if (null? result-range-list)
                      #f
                      (car result-range-list))))
        (if (and r-last
                 (<= (range-start r-now)  (+ (range-end r-last) 1))
                 (<= (range-start r-last) (+ (range-end r-now)  1)))
          (set-car! result-range-list
                    (make-range (min (range-start r-now) (range-start r-last))
                                (max (range-end   r-now) (range-end   r-last))))
          (push! result-range-list r-now)))
      ;; リストの残りをチェック
      (if (and (null? rest1) (null? rest2))
        (reverse result-range-list)
        (loop #f
              (if (null? rest1) #f (car rest1))
              (if (null? rest2) #f (car rest2))
              rest1
              rest2))))))


;; EastAsianWidth の読み込み
;;   ・wide-type-selector で選択したものだけを読み込む
;;   ・一部範囲を限定して読み込んでいるので注意
(define (read-east-asian-width)
  (define result-range-list '())
  (define (range-list-push! start end type)
    (let ((start (string->number start 16))
          (end   (string->number end   16))
          (type  (string->symbol type)))
      (when (and (memq type wide-type-selector)
                 ;(< start #xf0000) ; 一部範囲を限定
                 )
        (push! result-range-list (make-range start end)))))
  (for-each
   (lambda (line)
     (rxmatch-case line
       (#/^(\w+)\.\.(\w+)\s*;\s*(\w+)/ (#f start end type)
        (range-list-push! start end   type))
       (#/^(\w+)\s*;\s*(\w+)/          (#f start type)
        (range-list-push! start start type))))
   (generator->lseq read-line))
  (reverse result-range-list))

;; emoji-data の読み込み
;;   ・プロパティが Emoji のものだけを読み込む
;;   ・一部範囲を限定して読み込んでいるので注意
(define (read-emoji-data)
  (define result-range-list '())
  (define (range-list-push! start end type)
    (let ((start (string->number start 16))
          (end   (string->number end   16))
          (type  (string->symbol type)))
      (when (and (eq? type 'Emoji)
                 (>= start #x1000)) ; 一部範囲を限定
        (push! result-range-list (make-range start end)))))
  (for-each
   (lambda (line)
     (rxmatch-case line
       (#/^(\w+)\.\.(\w+)\s*;\s*(\w+)/ (#f start end type)
        (range-list-push! start end   type))
       (#/^(\w+)\s*;\s*(\w+)/          (#f start type)
        (range-list-push! start start type))))
   (generator->lseq read-line))
  (reverse result-range-list))

;; 結果データの書き出し
(define (write-result-data range-list)
  (define indent      (make-string (- result-data-indent 1) #\space))
  (define range-num   (length range-list))
  (define range-count 0)
  (for-each
   (lambda (r)
     (cond ((= range-count 0)
            (format #t "~a" indent))
           ((= (modulo range-count 3) 0)
            (format #t "~%~a" indent)))
     (format #t " { 0x~4,'0x, 0x~4,'0x }" (range-start r) (range-end r))
     (inc! range-count)
     (when (< range-count range-num)
       (format #t ",")))
   range-list)
  (format #t "~%"))

;; ファイル変換
(define (convert-file)
  (define result-range-list '())
  ;; EastAsianWidth の読み込み
  (when east-asian-width-file
    (let ((range-list '()))
      (with-input-from-file east-asian-width-file
        (lambda ()
          (set! range-list (compress-range-list
                            (sort-range-list (read-east-asian-width))))))
      (set! result-range-list (merge-range-list result-range-list range-list))))
  ;; emoji-data の読み込み
  (when emoji-data-file
    (let ((range-list '()))
      (with-input-from-file emoji-data-file
        (lambda ()
          (set! range-list (compress-range-list
                            (sort-range-list (read-emoji-data))))))
      (set! result-range-list (merge-range-list result-range-list range-list))))
  ;; ワイド文字範囲リストの追加設定をマージする
  (when wide-range-list-plus
    (set! result-range-list (merge-range-list result-range-list wide-range-list-plus)))
  ;; 結果データの書き出し
  (when result-data-file
    (with-output-to-file result-data-file
      (lambda ()
        (write-result-data result-range-list)))))

;; 使い方の表示
(define (usage out code)
  (format out "Usage: gosh gen_eaw_cdata.scm mode(=0-3)~%")
  (exit code))

;; メイン処理
(define (main args)
  (match args
    ((_ "0") ; テスト用
     (set! east-asian-width-file "EastAsianWidth.txt")
     (set! emoji-data-file       "emoji-data.txt")
     (set! result-data-file      "wide-test.txt")
     (set! wide-type-selector    '(F W))
     (set! wide-range-list-plus  '((#x10000 . #xeffff)))
     (set! result-data-indent    8)
     (convert-file))
    ((_ "1") ; Full Width 用
     (set! emoji-data-file       #f)
     (set! result-data-file      "wide-full.txt")
     (set! wide-type-selector    '(F W))
     (set! wide-range-list-plus  #f)
     (convert-file))
    ((_ "2") ; Ambiguous Width 用
     (set! emoji-data-file       #f)
     (set! result-data-file      "wide-ambiguous.txt")
     (set! wide-type-selector    '(A))
     (set! wide-range-list-plus  #f)
     (convert-file))
    ((_ "3") ; Emoji 用
     (set! east-asian-width-file #f)
     (set! emoji-data-file       "emoji-data.txt")
     (set! result-data-file      "wide-emoji.txt")
     (set! wide-type-selector    '())
     (set! wide-range-list-plus  #f)
     (convert-file))
    (_ (usage (current-error-port) 1)))
  0)

