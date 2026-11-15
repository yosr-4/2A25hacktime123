-- =====================================================================
--  HackTime - jury d'une competition (a executer UNE fois, connecte en hackathon, F5)
--    Competition (..., #id_jury)  ->  ID_JURY = employe de role JURY
-- =====================================================================

ALTER TABLE COMPETITION ADD (ID_JURY NUMBER);

ALTER TABLE COMPETITION ADD CONSTRAINT FK_COMPETITION_JURY
    FOREIGN KEY (ID_JURY) REFERENCES EMPLOYE (ID_EMPLOYE)
    ON DELETE SET NULL;

-- seul un employe de role JURY peut etre jury d'une competition
CREATE OR REPLACE TRIGGER TRG_COMPETITION_JURY
BEFORE INSERT OR UPDATE OF ID_JURY ON COMPETITION
FOR EACH ROW
WHEN (NEW.ID_JURY IS NOT NULL)
DECLARE
    v_role EMPLOYE.ROLE%TYPE;
BEGIN
    SELECT ROLE INTO v_role FROM EMPLOYE WHERE ID_EMPLOYE = :NEW.ID_JURY;
    IF v_role <> 'JURY' THEN
        RAISE_APPLICATION_ERROR(-20001, 'Cet employé n''est pas un jury.');
    END IF;
END;
/

COMMIT;

-- Verification :
-- DESC COMPETITION;   -> ID_JURY
-- SELECT constraint_name, constraint_type FROM user_constraints WHERE table_name = 'COMPETITION';
--                     -> FK_COMPETITION_JURY (R)
