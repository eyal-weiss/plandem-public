; Transport city-sequential-9nodes-100meterresolution-2trucks-6packages-ontario

(define (problem transport-city-sequential-9nodes-100meterresolution-2trucks-6packages-ontario)
 (:domain transport)
 (:objects
  Toronto - location
  Mississauga - location
  Hamilton - location
  Kitchener - location
  Brantford - location
  London - location
  Stratford - location
  Woodstock - location
  Barrie - location
  truck-1 - vehicle
  truck-2 - vehicle
;  truck-3 - vehicle
  package-1 - package
  package-2 - package
  package-3 - package
  package-4 - package
  package-5 - package
  package-6 - package
  capacity-0 - capacity-number
  capacity-1 - capacity-number
  capacity-2 - capacity-number
  capacity-3 - capacity-number
  capacity-4 - capacity-number
 )
 (:init
  (= (total-cost) 0)
  (capacity-predecessor capacity-0 capacity-1)
  (capacity-predecessor capacity-1 capacity-2)
  (capacity-predecessor capacity-2 capacity-3)
  (capacity-predecessor capacity-3 capacity-4)

  (road Toronto Mississauga)
  (= (road-length Toronto Mississauga) 275)

  (road Mississauga Toronto)
  (= (road-length Mississauga Toronto) 281)

  (road Toronto Barrie)
  (= (road-length Toronto Barrie) 1101)

  (road Barrie Toronto)
  (= (road-length Barrie Toronto) 1106)

  (road Mississauga Barrie)
  (= (road-length Mississauga Barrie) 1035)

  (road Barrie Mississauga)
  (= (road-length Barrie Mississauga) 1020)

  (road Barrie Kitchener)
  (= (road-length Barrie Kitchener) 1678)

  (road Kitchener Barrie)
  (= (road-length Kitchener Barrie) 1683)

  (road Mississauga Kitchener)
  (= (road-length Mississauga Kitchener) 852)

  (road Kitchener Mississauga)
  (= (road-length Kitchener Mississauga) 837)

  (road Mississauga Hamilton)
  (= (road-length Mississauga Hamilton) 479)

  (road Hamilton Mississauga)
  (= (road-length Hamilton Mississauga) 478)

  (road Hamilton Kitchener)
  (= (road-length Hamilton Kitchener) 657)

  (road Kitchener Hamilton)
  (= (road-length Kitchener Hamilton) 646)

  (road Hamilton Brantford)
  (= (road-length Hamilton Brantford) 408)

  (road Brantford Hamilton)
  (= (road-length Brantford Hamilton) 407)

  (road Brantford Kitchener)
  (= (road-length Brantford Kitchener) 522)

  (road Kitchener Brantford)
  (= (road-length Kitchener Brantford) 476)

  (road Brantford Woodstock)
  (= (road-length Brantford Woodstock) 442)

  (road Woodstock Brantford)
  (= (road-length Woodstock Brantford) 443)

  (road Kitchener Woodstock)
  (= (road-length Kitchener Woodstock) 562)

  (road Woodstock Kitchener)
  (= (road-length Woodstock Kitchener) 578)

  (road Kitchener Stratford)
  (= (road-length Kitchener Stratford) 506)

  (road Stratford Kitchener)
  (= (road-length Stratford Kitchener) 496)

  (road Stratford Woodstock)
  (= (road-length Stratford Woodstock) 376)

  (road Woodstock Stratford)
  (= (road-length Woodstock Stratford) 377)

  (road Stratford London)
  (= (road-length Stratford London) 607)

  (road London Stratford)
  (= (road-length London Stratford) 606)

  (road London Woodstock)
  (= (road-length London Woodstock) 552)

  (road Woodstock London)
  (= (road-length Woodstock London) 544)

  (at package-1 Toronto)
  (at package-2 Hamilton)
  (at package-3 London)
  (at package-4 Barrie)
  (at package-5 Brantford)
  (at package-6 Kitchener)
  (at truck-1 Kitchener)
  (capacity truck-1 capacity-2)
  (at truck-2 Mississauga)
  (capacity truck-2 capacity-4)
;  (at truck-3 Woodstock)
;  (capacity truck-3 capacity-1)
 )
 (:goal (and
  (at package-1 Stratford)
  (at package-2 Woodstock)
  (at package-3 Toronto)
  (at package-4 Brantford)
  (at package-5 Hamilton)
  (at package-6 London)
 ))
 (:metric minimize (total-cost))
)
