-- =====================================================================
--  HackTime - TOUTES les modifications de la base, en un seul script
--  SQL Developer : connexion hackathon -> Fichier > Ouvrir ce fichier -> F5
--  Chaque partie verifie d'abord si elle est deja faite :
--  on peut le lancer plusieurs fois, les donnees sont conservees.
-- =====================================================================
SET SERVEROUTPUT ON

DECLARE
    n NUMBER;

    -- compte de connexion : cree s'il n'existe pas, sinon mot de passe et acces mis a jour
    PROCEDURE compte(p_login VARCHAR2, p_nom VARCHAR2, p_role VARCHAR2, p_acces VARCHAR2) IS
        v_existe NUMBER;
        v_id     NUMBER;
    BEGIN
        SELECT COUNT(*) INTO v_existe FROM EMPLOYE WHERE LOWER(LOGIN) = LOWER(p_login);
        IF v_existe > 0 THEN
            EXECUTE IMMEDIATE 'UPDATE EMPLOYE SET LOGIN = :1, MDP = :2, ACCES = :3 WHERE LOWER(LOGIN) = LOWER(:4)'
                USING p_login, p_login, p_acces, p_login;
            DBMS_OUTPUT.PUT_LINE('   compte ' || p_login || ' mis a jour -> ' || p_acces);
        ELSE
            SELECT NVL(MAX(ID_EMPLOYE), 0) + 1 INTO v_id FROM EMPLOYE;
            EXECUTE IMMEDIATE 'INSERT INTO EMPLOYE (ID_EMPLOYE, NOM, PRENOM, EMAIL, ROLE, LOGIN, MDP, ACCES)
                               VALUES (:1, :2, :3, :4, :5, :6, :7, :8)'
                USING v_id, p_nom, p_nom, p_login || '@hacktime.tn', p_role, p_login, p_login, p_acces;
            DBMS_OUTPUT.PUT_LINE('   compte ' || p_login || ' cree -> ' || p_acces);
        END IF;
    END;
BEGIN
    -- ------------------------------------------------------------------
    -- 1. Nom de competition unique
    -- ------------------------------------------------------------------
    SELECT COUNT(*) INTO n FROM user_constraints WHERE constraint_name = 'UQ_COMPETITION_NOM';
    IF n = 0 THEN
        SELECT COUNT(*) INTO n FROM (SELECT NOM FROM COMPETITION GROUP BY NOM HAVING COUNT(*) > 1);
        IF n > 0 THEN
            DBMS_OUTPUT.PUT_LINE('1. ATTENTION : noms de competition en double, corrigez-les puis relancez.');
        ELSE
            EXECUTE IMMEDIATE 'ALTER TABLE COMPETITION ADD CONSTRAINT UQ_COMPETITION_NOM UNIQUE (NOM)';
            DBMS_OUTPUT.PUT_LINE('1. Nom de competition rendu unique.');
        END IF;
    ELSE
        DBMS_OUTPUT.PUT_LINE('1. Nom unique : deja fait.');
    END IF;

    -- ------------------------------------------------------------------
    -- 2. Jury d'une competition : COMPETITION.ID_JURY -> EMPLOYE
    -- ------------------------------------------------------------------
    SELECT COUNT(*) INTO n FROM user_tab_columns WHERE table_name = 'COMPETITION' AND column_name = 'ID_JURY';
    IF n = 0 THEN
        EXECUTE IMMEDIATE 'ALTER TABLE COMPETITION ADD (ID_JURY NUMBER)';
        DBMS_OUTPUT.PUT_LINE('2. Colonne COMPETITION.ID_JURY ajoutee.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('2. Colonne ID_JURY : deja faite.');
    END IF;
    SELECT COUNT(*) INTO n FROM user_constraints WHERE constraint_name = 'FK_COMPETITION_JURY';
    IF n = 0 THEN
        EXECUTE IMMEDIATE 'ALTER TABLE COMPETITION ADD CONSTRAINT FK_COMPETITION_JURY
                           FOREIGN KEY (ID_JURY) REFERENCES EMPLOYE (ID_EMPLOYE) ON DELETE SET NULL';
        DBMS_OUTPUT.PUT_LINE('   cle FK_COMPETITION_JURY ajoutee.');
    END IF;
    EXECUTE IMMEDIATE q'[
        CREATE OR REPLACE TRIGGER TRG_COMPETITION_JURY
        BEFORE INSERT OR UPDATE OF ID_JURY ON COMPETITION
        FOR EACH ROW
        WHEN (NEW.ID_JURY IS NOT NULL)
        DECLARE
            v_role EMPLOYE.ROLE%TYPE;
        BEGIN
            SELECT ROLE INTO v_role FROM EMPLOYE WHERE ID_EMPLOYE = :NEW.ID_JURY;
            IF v_role <> 'JURY' THEN
                RAISE_APPLICATION_ERROR(-20001, 'Cet employe n''est pas un jury.');
            END IF;
        END;]';

    -- ------------------------------------------------------------------
    -- 3. Acces par gestion : EMPLOYE.ACCES + comptes de connexion
    -- ------------------------------------------------------------------
    SELECT COUNT(*) INTO n FROM user_tab_columns WHERE table_name = 'EMPLOYE' AND column_name = 'ACCES';
    IF n = 0 THEN
        EXECUTE IMMEDIATE 'ALTER TABLE EMPLOYE ADD (ACCES VARCHAR2(20))';
        EXECUTE IMMEDIATE 'ALTER TABLE EMPLOYE ADD CONSTRAINT CK_EMPLOYE_ACCES CHECK (ACCES IN '
            || '(''TOUT'', ''COMPETITIONS'', ''EQUIPES'', ''PARTICIPANTS'', ''ATELIERS'', ''EMPLOYES''))';
        DBMS_OUTPUT.PUT_LINE('3. Colonne EMPLOYE.ACCES ajoutee.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('3. Colonne ACCES : deja faite.');
    END IF;
    compte('hacktime123', 'Admin',   'ADMINISTRATEUR', 'TOUT');
    compte('yosr123',     'Yosr',    'ORGANISATEUR',   'COMPETITIONS');
    compte('mariem123',   'Mariem',  'ORGANISATEUR',   'EQUIPES');
    compte('yasmine123',  'Yasmine', 'ORGANISATEUR',   'PARTICIPANTS');
    compte('adam123',     'Adam',    'ORGANISATEUR',   'ATELIERS');
    compte('mohamed123',  'Mohamed', 'ORGANISATEUR',   'EMPLOYES');

    -- ------------------------------------------------------------------
    -- 4. Sequences recalees apres le plus grand ID (evite l'erreur ORA-00001)
    -- ------------------------------------------------------------------
    FOR s IN (SELECT 'SEQ_COMPETITION' seq, 'COMPETITION' tab, 'ID_COMPETITION' col FROM DUAL UNION ALL
              SELECT 'SEQ_EMPLOYE', 'EMPLOYE', 'ID_EMPLOYE' FROM DUAL UNION ALL
              SELECT 'SEQ_ATELIER', 'ATELIER', 'ID_ATELIER' FROM DUAL UNION ALL
              SELECT 'SEQ_EQUIPE', 'EQUIPE', 'ID_EQUIPE' FROM DUAL UNION ALL
              SELECT 'SEQ_PARTICIPANT', 'PARTICIPANT', 'ID_PARTICIPANT' FROM DUAL) LOOP
        SELECT COUNT(*) INTO n FROM user_sequences WHERE sequence_name = s.seq;
        IF n > 0 THEN
            EXECUTE IMMEDIATE 'SELECT NVL(MAX(' || s.col || '), 0) + 1 FROM ' || s.tab INTO n;
            EXECUTE IMMEDIATE 'DROP SEQUENCE ' || s.seq;
            EXECUTE IMMEDIATE 'CREATE SEQUENCE ' || s.seq || ' START WITH ' || n || ' INCREMENT BY 1 NOCACHE';
        END IF;
    END LOOP;
    DBMS_OUTPUT.PUT_LINE('4. Sequences recalees.');
END;
/

COMMIT;

-- =====================================================================
-- Verification (Ctrl+Entree sur chaque ligne) :
-- SELECT LOGIN, MDP, ROLE, ACCES FROM EMPLOYE ORDER BY ID_EMPLOYE;
-- DESC COMPETITION;            -> ID_JURY
-- SELECT constraint_name, constraint_type FROM user_constraints WHERE table_name = 'COMPETITION';
-- =====================================================================
