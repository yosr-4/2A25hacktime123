-- =====================================================================
--  HackTime - participants de démonstration (SANS équipe)
--  Pour montrer au prof : composition intelligente + participants à risque.
--  Exécutez chaque instruction une par une (Ctrl+Entrée) puis COMMIT.
--  Codes compétence : DEV, DESIGN, DATA, MARKETING, GESTION   |   Niveau : 1, 2, 3
-- =====================================================================
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (101, 'Ben Salah', 'Amine', 'amine.bensalah@esprit.tn', '20111222', 'DEV', 3, 'INSCRIT', SYSDATE - 12, 2);
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (102, 'Trabelsi', 'Sarra', 'sarra.trabelsi@gmail.com', '22333444', 'DESIGN', 2, 'INSCRIT', SYSDATE - 1, 0);
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (103, 'Gharbi', 'Omar', 'omar.gharbi@esprit.tn', '22110099', 'DATA', 3, 'INSCRIT', SYSDATE - 4, 0);
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (104, 'Saidi', 'Nour', 'nour.saidi@gmail.com', '44556677', 'MARKETING', 1, 'INSCRIT', SYSDATE - 9, 1);
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (105, 'Mejri', 'Hela', 'hela.mejri@esprit.tn', '55667788', 'DEV', 2, 'INSCRIT', SYSDATE - 2, 0);
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (106, 'Jlassi', 'Karim', 'karim.jlassi@gmail.com', '98765432', 'GESTION', 2, 'INSCRIT', SYSDATE - 7, 1);
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (107, 'Hamdi', 'Salma', 'salma.hamdi@esprit.tn', '23456789', 'DATA', 1, 'INSCRIT', SYSDATE - 11, 2);
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (108, 'Bouzid', 'Yassine', 'yassine.bouzid@gmail.com', '27654321', 'DEV', 1, 'INSCRIT', SYSDATE - 3, 0);
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (109, 'Chaabane', 'Ines', 'ines.chaabane@esprit.tn', '29876543', 'DESIGN', 3, 'INSCRIT', SYSDATE - 5, 0);
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (110, 'Karray', 'Mehdi', 'mehdi.karray@gmail.com', '21987654', 'MARKETING', 2, 'INSCRIT', SYSDATE - 6, 0);
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (111, 'Dridi', 'Rim', 'rim.dridi@esprit.tn', '24681357', 'GESTION', 3, 'INSCRIT', SYSDATE - 1, 0);
INSERT INTO participant (id_participant, nom, prenom, email, telephone, competence, niveau, statut, date_inscription, nb_relances) VALUES (112, 'Ayari', 'Walid', 'walid.ayari@gmail.com', '26897531', 'DEV', 2, 'INSCRIT', SYSDATE - 8, 1);
COMMIT;

-- Pour supprimer ces participants de démonstration :
-- DELETE FROM participant WHERE id_participant BETWEEN 101 AND 112;
-- COMMIT;
