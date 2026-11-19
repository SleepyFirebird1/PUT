(define (problem merry-problem)
  (:domain merry)
  (:objects
    a b c d e f g h i - pokoj
    zielony czerwony pomaranczowy rozowy niebieski - kolor
    b1 b2 b3 b4 b5 b6 b7 b8 b9 b10 b11 b12 b13 b14 - kulka
    p11 p12 p13 p21 p22 p23 p31 p32 p33 - pozycja
    k1 k2 k3 k4 k5 k6 k7 k8 - klocek
  )

  (:init
    ;; Pozycja poczatkowa Merry'ego (pokoj F)
    (merry-w f)

    ;; Pomieszczenie z lamiglowka (A)
    (puzzle-w a)

    ;; Rozmieszczenie kulek w piwnicy:
    ;; A: 1 zielona
    (kulka-w a b1) (jest-koloru b1 zielony)

    ;; B: 1 rozowa
    (kulka-w b b2) (jest-koloru b2 rozowy)

    ;; C: 1 pomaranczowa, 1 niebieska
    (kulka-w c b3) (jest-koloru b3 pomaranczowy)
    (kulka-w c b4) (jest-koloru b4 niebieski)

    ;; D: 1 zielona, 1 czerwona
    (kulka-w d b5) (jest-koloru b5 zielony)
    (kulka-w d b6) (jest-koloru b6 czerwony)

    ;; E: 1 niebieska, 1 zielona
    (kulka-w e b7) (jest-koloru b7 niebieski)
    (kulka-w e b8) (jest-koloru b8 zielony)

    ;; F: 2 niebieskie
    (kulka-w f b9) (jest-koloru b9 niebieski)
    (kulka-w f b10) (jest-koloru b10 niebieski)

    ;; H: 1 zielona, 1 czerwona
    (kulka-w h b11) (jest-koloru b11 zielony)
    (kulka-w h b12) (jest-koloru b12 czerwony)

    ;; I: 1 pomaranczowa, 1 niebieska
    (kulka-w i b13) (jest-koloru b13 pomaranczowy)
    (kulka-w i b14) (jest-koloru b14 niebieski)

    ;; Polaczenia drzwiowe:
    ;; A -> B pomaranczowy (jednokierunkowe)
    (drzwi a b pomaranczowy)

    ;; A <-> C czerwony (dwukierunkowe)
    (drzwi a c czerwony)
    (drzwi c a czerwony)

    ;; B -> D pomaranczowy (jednokierunkowe)
    (drzwi b d pomaranczowy)

    ;; C <-> E zielony (dwukierunkowe)
    (drzwi c e zielony)
    (drzwi e c zielony)

    ;; D <-> F zielony (dwukierunkowe)
    (drzwi d f zielony)
    (drzwi f d zielony)

    ;; E <-> F niebieski (dwukierunkowe)
    (drzwi e f niebieski)
    (drzwi f e niebieski)

    ;; E <-> H czerwony (dwukierunkowe)
    (drzwi e h czerwony)
    (drzwi h e czerwony)

    ;; F <-> I niebieski (dwukierunkowe)
    (drzwi f i niebieski)
    (drzwi i f niebieski)

    ;; H <-> I niebieski (dwukierunkowe)
    (drzwi h i niebieski)
    (drzwi i h niebieski)

    ;; F -> G rozowy (jednokierunkowe)
    (drzwi f g rozowy)

    ;; Sasiedztwo pol w siatce 3x3 (pion/poziom)
    (sasiednia p11 p12) (sasiednia p12 p11)
    (sasiednia p12 p13) (sasiednia p13 p12)
    (sasiednia p21 p22) (sasiednia p22 p21)
    (sasiednia p22 p23) (sasiednia p23 p22)
    (sasiednia p31 p32) (sasiednia p32 p31)
    (sasiednia p32 p33) (sasiednia p33 p32)

    (sasiednia p11 p21) (sasiednia p21 p11)
    (sasiednia p12 p22) (sasiednia p22 p12)
    (sasiednia p13 p23) (sasiednia p23 p13)
    (sasiednia p21 p31) (sasiednia p31 p21)
    (sasiednia p22 p32) (sasiednia p32 p22)
    (sasiednia p23 p33) (sasiednia p33 p23)

    ;; Stan poczatkowy ukladanki 3x3:
    ;; 5 4 1
    ;; 8 _ 3
    ;; 7 2 6
    (na k5 p11)
    (na k4 p12)
    (na k1 p13)
    (na k8 p21)
    (puste p22)
    (na k3 p23)
    (na k7 p31)
    (na k2 p32)
    (na k6 p33)
  )

  (:goal (and
    ;; Dotrzec do pokoju z drabina
    (merry-w g)

    ;; Ukladanka rozwiazana:
    ;; 1 2 3
    ;; 4 5 6
    ;; 7 8 _
    (na k1 p11)
    (na k2 p12)
    (na k3 p13)
    (na k4 p21)
    (na k5 p22)
    (na k6 p23)
    (na k7 p31)
    (na k8 p32)
    (puste p33)
  ))
)