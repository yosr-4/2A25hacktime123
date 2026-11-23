-- =====================================================================
--  HackTime - colonnes ajoutées à la table PARTICIPANT
--  OPTIONNEL : l'application les crée AUTOMATIQUEMENT au démarrage.
--  Utilisez ce script seulement si vous préférez le faire à la main.
--  (Ignorez l'erreur ORA-01430 "colonne déjà existante" si elle apparaît.)
--  Exécutez chaque instruction une par une (Ctrl+Entrée) dans SQL Developer.
-- =====================================================================
ALTER TABLE participant ADD (id_equipe NUMBER);
ALTER TABLE participant ADD (competence VARCHAR2(30));
ALTER TABLE participant ADD (niveau NUMBER(1) DEFAULT 1);
ALTER TABLE participant ADD (statut VARCHAR2(20) DEFAULT 'INSCRIT');
ALTER TABLE participant ADD (date_inscription DATE DEFAULT SYSDATE);
ALTER TABLE participant ADD (nb_relances NUMBER DEFAULT 0);
-- scénario Arduino (barrière d'accès) : date du dernier pointage (ajoutée aussi automatiquement par l'application)
ALTER TABLE participant ADD (date_pointage DATE);
COMMIT;

-- Table d'historique (ce que chaque participant a fait) : créée aussi automatiquement par l'application.
CREATE TABLE historique_participant (id_participant NUMBER, date_action DATE, action VARCHAR2(30), details VARCHAR2(100));
