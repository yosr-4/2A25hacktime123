-- =====================================================================
--  HackTime - association GERER (Employe n,n Competition)
--    Gerer (#id_employe, #id_competition)
--  À exécuter une seule fois (SQL Developer, connecté en hackathon, F5)
--  sur une base créée avec hacktime_commune.sql.
--  Les responsables actuels (COMPETITION.ID_EMPLOYE) sont recopiés dans
--  GERER, puis la colonne ID_EMPLOYE est retirée de COMPETITION.
-- =====================================================================

CREATE TABLE GERER (
    ID_EMPLOYE      NUMBER NOT NULL
                    CONSTRAINT FK_GERER_EMPLOYE REFERENCES EMPLOYE (ID_EMPLOYE)
                    ON DELETE CASCADE,
    ID_COMPETITION  NUMBER NOT NULL
                    CONSTRAINT FK_GERER_COMPETITION REFERENCES COMPETITION (ID_COMPETITION)
                    ON DELETE CASCADE,
    CONSTRAINT PK_GERER PRIMARY KEY (ID_EMPLOYE, ID_COMPETITION)
);

INSERT INTO GERER (ID_EMPLOYE, ID_COMPETITION)
SELECT ID_EMPLOYE, ID_COMPETITION FROM COMPETITION WHERE ID_EMPLOYE IS NOT NULL;

ALTER TABLE COMPETITION DROP COLUMN ID_EMPLOYE;

COMMIT;

-- Vérification :
-- SELECT * FROM GERER;
