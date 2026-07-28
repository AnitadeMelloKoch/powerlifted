(define (domain minecraft-blocks)
    (:requirements :strips)

    (:types agent wood stone iron gold diamond)

    (:predicates
        (alive ?ag - agent)
        (has_wood ?ag - agent)
        (has_stone ?ag - agent)
        (has_iron ?ag - agent)
        (has_gold ?ag - agent)
        (has_diamond ?ag - agent)
        (has_wood_pickaxe ?ag - agent)
        (has_stone_pickaxe ?ag - agent)
        (has_iron_pickaxe ?ag - agent)
        (has_gold_pickaxe ?ag - agent)
        (has_diamond_pickaxe ?ag - agent)

        (wood_collected ?w - wood)
        (wood_not_collected ?w - wood)

        (stone_collected ?s - stone)
        (stone_not_collected ?s - stone)

        (iron_collected ?i - iron)
        (iron_not_collected ?i - iron)

        (gold_collected ?g - gold)
        (gold_not_collected ?g - gold)

        (diamond_collected ?d - diamond)
        (diamond_not_collected ?d - diamond)
    )

    (:action collect_wood_block
        :parameters (?ag - agent ?w - wood)
        :precondition (and
            (wood_not_collected ?w)
            (alive ?ag)
        )
        :effect (and
            (has_wood ?ag)
            (wood_collected ?w)
            (not (wood_not_collected ?w))
        )
    )

    (:action collect_stone_block
        :parameters (?ag - agent ?s - stone)
        :precondition (and
            (or
                (has_wood_pickaxe ?ag)
                (has_stone_pickaxe ?ag)
                (has_gold_pickaxe ?ag)
                (has_diamond_pickaxe ?ag)
            )
            (stone_not_collected ?s)
        )
        :effect (and
            (has_stone ?ag)
            (stone_collected ?s)
            (not (stone_not_collected ?s))
        )
    )

    (:action mine_iron_ore
        :parameters (?ag - agent ?i - iron)
        :precondition (and
            (or 
                (has_stone_pickaxe ?ag)
                (has_iron_pickaxe ?ag)
                (has_gold_pickaxe ?ag)
                (has_diamond_pickaxe ?ag)
            )
            (iron_not_collected ?i)
        )
        :effect (and
            (has_iron ?ag)
            (iron_collected ?i)
            (not (iron_not_collected ?i))
        )
    )

    (:action mine_gold_ore
        :parameters (?ag - agent ?g - gold)
        :precondition (and
            (or
                (has_iron_pickaxe ?ag)
                (has_gold_pickaxe ?ag)
                (has_diamond_pickaxe ?ag)
            )
            (gold_not_collected ?g)
        )
        :effect (and
            (has_gold ?ag)
            (gold_collected ?g)
            (not (gold_not_collected ?g))
        )
    )

    (:action mine_diamond_ore
        :parameters (?ag - agent ?d - diamond)
        :precondition (and
            (or
                (has_iron_pickaxe ?ag)
                (has_gold_pickaxe ?ag)
                (has_diamond_pickaxe ?ag)
            )
            (diamond_not_collected ?d)
        )
        :effect (and
            (has_diamond ?ag)
            (diamond_collected ?d)
            (not (diamond_not_collected ?d))
        )
    )

    (:action craft_wood_pickaxe
        :parameters (?ag - agent
                     ?w1 - wood
                     ?w2 - wood
                     ?w3 - wood
                     ?w4 - wood
                     ?w5 - wood)
        :precondition (and
            (has_wood ?ag)
            (wood_collected ?w1)
            (wood_collected ?w2)
            (wood_collected ?w3)
            (wood_collected ?w4)
            (wood_collected ?w5)
            (not (= ?w1 ?w2))
            (not (= ?w1 ?w3))
            (not (= ?w1 ?w4))
            (not (= ?w1 ?w5))
            (not (= ?w2 ?w3))
            (not (= ?w2 ?w4))
            (not (= ?w2 ?w5))
            (not (= ?w3 ?w4))
            (not (= ?w3 ?w5))
            (not (= ?w4 ?w5))
        )
        :effect (and
            (not (has_wood ?ag))
            (not (wood_collected ?w1))
            (not (wood_collected ?w2))
            (not (wood_collected ?w3))
            (not (wood_collected ?w4))
            (not (wood_collected ?w5))
            (has_wood_pickaxe ?ag)
        )
    )

    (:action craft_stone_pickaxe
        :parameters (?ag - agent
                     ?w1 - wood
                     ?w2 - wood
                     ?s1 - stone
                     ?s2 - stone
                     ?s3 - stone)
        :precondition (and
            (has_wood ?ag)
            (has_stone ?ag)
            (wood_collected ?w1)
            (wood_collected ?w2)
            (stone_collected ?s1)
            (stone_collected ?s2)
            (stone_collected ?s3)
            (not (= ?w1 ?w2))
            (not (= ?s1 ?s2))
            (not (= ?s1 ?s3))
            (not (= ?s2 ?s3))
        )
        :effect (and
            (not (has_wood ?ag))
            (not (has_stone ?ag))
            (not (wood_collected ?w1))
            (not (wood_collected ?w2))
            (not (stone_collected ?s1))
            (not (stone_collected ?s2))
            (not (stone_collected ?s3))
            (has_stone_pickaxe ?ag)
        )
    )

    (:action craft_iron_pickaxe
        :parameters (?ag - agent
                     ?w1 - wood
                     ?w2 - wood 
                     ?i1 - iron 
                     ?i2 - iron
                     ?i3 - iron)
        :precondition (and
            (has_wood ?ag)
            (has_iron ?ag)
            (wood_collected ?w1)
            (wood_collected ?w2)
            (iron_collected ?i1)
            (iron_collected ?i2)
            (iron_collected ?i3)
            (not (= ?w1 ?w2))
            (not (= ?i1 ?i2))
            (not (= ?i1 ?i3))
            (not (= ?i2 ?i3))
        )
        :effect (and
            (not (has_wood ?ag))
            (not (has_iron ?ag))
            (not (wood_collected ?w1))
            (not (wood_collected ?w2))
            (not (iron_collected ?i1))
            (not (iron_collected ?i2))
            (not (iron_collected ?i3))
            (has_iron_pickaxe ?ag)
        )
    )

    (:action craft_gold_pickaxe
        :parameters (?ag - agent
                     ?w1 - wood
                     ?w2 - wood
                     ?g1 - gold
                     ?g2 - gold
                     ?g3 - gold)
        :precondition (and
            (has_wood ?ag)
            (has_gold ?ag)
            (wood_collected ?w1)
            (wood_collected ?w2)
            (gold_collected ?g1)
            (gold_collected ?g2)
            (gold_collected ?g3)
            (not (= ?w1 ?w2))
            (not (= ?g1 ?g2))
            (not (= ?g1 ?g3))
            (not (= ?g2 ?g3))
        )
        :effect (and
            (not (has_wood ?ag))
            (not (has_gold ?ag))
            (not (wood_collected ?w1))
            (not (wood_collected ?w2))
            (not (gold_collected ?g1))
            (not (gold_collected ?g2))
            (not (gold_collected ?g3))
            (has_gold_pickaxe ?ag)
        )
    )

    (:action craft_diamond_pickaxe
        :parameters (?ag - agent
                     ?w1 - wood
                     ?w2 - wood
                     ?d1 - diamond
                     ?d2 - diamond
                     ?d3 - diamond)
        :precondition (and
            (has_wood ?ag)
            (has_diamond ?ag)
            (wood_collected ?w1)
            (wood_collected ?w2)
            (diamond_collected ?d1)
            (diamond_collected ?d2)
            (diamond_collected ?d3)
            (not (= ?w1 ?w2))
            (not (= ?d1 ?d2))
            (not (= ?d1 ?d3))
            (not (= ?d2 ?d3))
        )
        :effect (and
            (not (has_wood ?ag))
            (not (has_diamond ?ag))
            (not (wood_collected ?w1))
            (not (wood_collected ?w2))
            (not (diamond_collected ?d1))
            (not (diamond_collected ?d2))
            (not (diamond_collected ?d3))
            (has_diamond_pickaxe ?ag)
        )
    )

)