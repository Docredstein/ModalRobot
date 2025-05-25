#import "@preview/typographix-polytechnique-reports:0.1.6" as template

// Defining variables for the cover page and PDF metadata
// Main title on cover page
#let title = [Rapport de ModAL
]
// Subtitle on cover page
#let subtitle = "PHY43M06EP: Robotique"
// Logo on cover page
#let logo = none // instead of none set to image("path/to/my-logo.png")
#let logo-horizontal = true // set to true if the logo is squared or horizontal, set to false if not
// Short title on headers
#let short-title = "Rapport de ModAL"
#let author = "FRAMBOURT Mateïs"
#let date-start = datetime(year: 2024, month: 06, day: 05)
#let date-end = datetime(year: 2024, month: 09, day: 05)
// Set to true for bigger margins and so on (good luck with your report)
#let despair-mode = false

#set text(lang: "fr")

// Set document metadata
#set document(title: title, author: author, date: datetime.today())
#show: template.apply.with(despair-mode: despair-mode)

// Cover page
#template.cover.cover(title, author, date-start, date-end, subtitle: subtitle, logo: logo, logo-horizontal: logo-horizontal)
#pagebreak()

// Acknowledgements
/*#heading(level: 1, numbering: none, outlined: false)[Remerciements]
#lorem(250)
#pagebreak()
*/
// Executive summary
/*#heading(level: 1, numbering: none, outlined: false)[Executive summary]
#lorem(300)
#pagebreak()
*/
// Table of contents
#outline(title: [Table des matières], indent: 1em, depth: 2) 

// Defining header and page numbering (will pagebreak)
#show: template.page.apply-header-footer.with(short-title: short-title)

// Introduction
#heading(level: 1, numbering: none)[Introduction]
Je suis arrivé dans ce modal un peu par hasard. Au départ je m'étais inscrit pour le ModAL FPGA car c'était quelque chose qui me semble très versatile mais un peu obscur pour le découvrir en auto-didacte. C'est donc avec surprise que j'apprend la disparition de ce ModAL dans une fusion avec le ModAL de Robotique. Au final, ça ne m'a pas trop géné car c'était mon deuxième voeu. J'avais une légère expérience en robotique. En effet, j'ai participé de 2012 à 2021 aux compétitions de robotique junior (robot radioguidés) de planêtes sciences et j'aide encore marginalement dans l'équipe familiale. J'avais donc une certaine connaissance des composants et de leurs utilisations mais je ne connaissais presque rien dans le contrôle et la commande haut niveau. Mon objectifs dans ce cours était donc d'apprendre cette partie pour pouvoir ensuite l'appliquer au Binet X-Robot qui est en manque crucial de RH et de compétence. 
#linebreak()
D'un point de vue plus académiques, l'objectif final est de réaliser un robot "tank" pour pouvoir travailler sur l'anticipation de mouvement de l'adversaire. l'objectif était donc de réussir à pointer une cible sur l'adversaire tout en se déplaçant dans un environnement encombré. Pour cela nous avions à notre disposition : #list(
  [Roues holonomes],
  [Moteurs avec encodeurs],
  [Raspberry pi 4],
  [Cartes moteurs I²C Groves],
  [Caméra grand angle 5MP],
  [Servomoteurs]
)
Cependant, avant de réaliser le projet final il est nécessaire de réaliser un suivi de ligne fonctionnel. Dans ce sens j'ai passé (malheureusement) la majeur partie des séances (8/10) sur ce projet initial.
#figure(
image("./source/global_low.jpg",fit: "contain",width:60% ),
caption: "Vue d'ensemble du robot final",
)




#pagebreak()

// Here goes the main content
= Base commune
== Base mécanique
Dans l'optique de pouvoir réutiliser un maximum de composants (tant mécaniques que logiciels), J'ai commencé par modéliser précisément les composants à ma disposition. Ensuite j'ai dessiné les supports des moteurs pour avoir une emprise au sol minimale (ils ont ensuite été imprimés au DrahiX). Enfin j'ai dessiné la plaque principale qui devait permettre d'avoir tous les composants à plats avec l'ensemble des trous déjà percés (elle a été découpée au laser du DrahiX). Cependant, la breadboard et la batterie ont été oubliées ce qui a mené à un montage relativement complexe à suivre et à modifier. L'objectifs n'a donc pas complètement été rempli. #linebreak()
J'ai choisi d'utiliser 3 moteurs en triangles pour 2 raisons : #list(
  [Il y a 3 degrés de liberté sur le robot donc avoir 3 moteurs pourrait donc nous permettre de facilement faire de la cinématique inverse exacte (ce n'est pas la méthode qui a finalement été utilisée)],
  [Les placers à 120° permets d'avoir une symétrie et donc on peut avoir plusieurs "face avant" du robot et il suffit juste de faire un décalage au niveau des consignes pour utiliser par exemple un autre outil]
)
D'autre part, la caméra est montée sur un pivot car elle possède un grand angle de vue. Il est donc possible de régler l'étendue du champ de vision en inclinant la caméra pour qu'elle ne soit pas perturbée par des objets extérieurs. 
#figure(
  image("./source/général_iso.png",fit:"contain",width: 60%),
  caption: "Vue isométrique de l'assemblage basique"
)

== Encodeurs
Rapidement, les encodeurs sont des paires de capteur à effet hall en quadrature de phase autour d'un aimant à 14 pôles radiaux de manières à ce qu'ils donnent 7 périodes de la @encoder par rotation.
Pour interprêter le signal, j'ai fait une librarie CMake qui attache une interruption aux 2 pins de l'encodeur et appelle une fonction très simple : 
```cpp uint8_t phase = identifier_phase();
    if (phase == (lastPhase + 1) % 4)
    {
        pos++;
    }
    else if (phase == (lastPhase - 1) % 4)
    {
        pos--;
    }
    lastPhase = phase; ```
et pour identifier la phase on fait appel à une table de correspondance : 
```cpp uint8_t const LUT[] = {0, 1, 3, 2};
    return LUT[2 * digitalRead(pinA) + digitalRead(pinB)];```
J'ai essayé pendant plusieurs séances d'implémenter cette librairie de manière conforme à la POO mais je n'arrivais pas à faire une fonction statique qui appelle une méthode. Finalement, j'ai réussi à l'implémenter pour une autre librairie dont je parlerais plus loin.
#linebreak()
Globalement, le problème principal de cette mesure est qu'elle donne précisément la position mais pas la vitesse. Il faut ensuite dériver numériquement ce qui ne donne pas de très bon résultat car la position est une fonction en escalier. Or je souhaitais asservir les moteurs en vitesse ce qui rend l'asservissement un peu instable. 
#figure(
  image("./source/encoder.png",fit: "contain",width: 60%),
  caption: "Signal en sortie des encodeurs (© Maxon)" 
)<encoder>
== PID
J'ai ensuite créé une librairie CMake générique pour les PID, c'est-à-dire un contrôleur générique à trois composante (proportionnelle, intégrale, dérivée). Celui-ci devait permettre d'avoir un contrôle précis et réactif mais les premiers moteurs était trop peu puissant et ne reposait presque que sur la composante intégrale. En effet, même en rêglant la composante proportionnelle à la limite de l'instabilité, la valeur atteinte en vitesse était très éloignée de la valeur désirée. D'autre part, l'entrée du moteur étant proportionnelle à sa vitesse, il fallait "intégrer" la commande envoyée au moteur. Dans le cas contraire, dès que l'erreur est nulle, la consigne envoyée au moteur est nulle et donc le moteur s'arrête. Ce n'est bien sûr pas le comportement voulu. Dans ce sens, l'ajout d'une commande constante au moteur permet au PID de compenser l'erreur que celle ci aurait seule. (On approxime donc le moteur à une réponse linéaire à la consigne et on corrige l'erreur d'approximation par PID). Cependant, même avec toute ces modifications, le moteur ne répondait pas correctement à faible commande. En effet, les effets de frictions sèches étaient trop important. Le modèle a donc été adapté pour les prendres en comptes (en augmentant la consigne artificiellement à faible commande). Pour limiter l'inertie de la composante intégrale, l'intégrale sature à partir d'une limite arbitraire.
#figure(
  image("./source/asservissement.png",fit:"contain"),
  caption: "Asservissement de chaque moteur"
)
== Roues holonomes
Les roues holonomes sont des roues qui n'imposent pas de contrainte normale à la roue. C'est dire qu'elle glisse sur la direction normale à la roue. On peut donc considérer uniquement comme un ajout de vecteur (il faut cependant )

 #figure(
  image("./source/modal géométries.png",fit: "contain",width: 80%),
  caption: "Vecteurs considérés pour les roues holonomes"
 )



#pagebreak()


= Robot suiveur



#pagebreak()

= Robot tank
@PP
#pagebreak()


// Conclusion
#heading(level: 1, numbering: none)[Conclusion]
#lorem(200)

// Bibliography (if necessary)
// #pagebreak()
// #bibliography("path-to-file.bib")+
#pagebreak()
#bibliography("./source/bibliography.bib")

// Annexe
#pagebreak() 
#show: template.heading.appendix.with(title: "Annexe")
= Fiche d'évaluation du stagiaire
Yeah j'ai eu que des A partout trop bien, je suis un.e super stagiaire.

