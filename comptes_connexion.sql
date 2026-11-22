-- =====================================================================
--  HackTime - comptes de connexion (table EMPLOYE : LOGIN, MDP, ROLE, ACCES)
--  A executer avec F5 dans SQL Developer, connecte en hackathon.
--  On peut le relancer sans risque : chaque compte est cree ou mis a jour.
--
--  Nouvelle colonne ACCES = la gestion que le compte peut ouvrir :
--    TOUT          -> toute l'application (administrateur)
--    COMPETITIONS / EQUIPES / PARTICIPANTS / ATELIERS / EMPLOYES -> cette gestion seulement
--  ACCES vide : ADMINISTRATEUR -> TOUT, autre role -> EMPLOYES.
-- =====================================================================
SET SERVEROUTPUT ON

DECLARE
    n NUMBER;

    -- cree le compte s'il n'existe pas, sinon met a jour son mot de passe et son acces
    PROCEDURE compte(p_login VARCHAR2, p_nom VARCHAR2, p_prenom VARCHAR2,
                     p_role VARCHAR2, p_acces VARCHAR2) IS
        v_existe NUMBER;
        v_id     NUMBER;
    BEGIN
        SELECT COUNT(*) INTO v_existe FROM EMPLOYE WHERE LOGIN = p_login;
        IF v_existe > 0 THEN
            EXECUTE IMMEDIATE 'UPDATE EMPLOYE SET MDP = :1, ACCES = :2 WHERE LOGIN = :3'
                USING p_login, p_acces, p_login;
            DBMS_OUTPUT.PUT_LINE('Compte ' || p_login || ' mis a jour (' || p_acces || ').');
        ELSE
            SELECT NVL(MAX(ID_EMPLOYE), 0) + 1 INTO v_id FROM EMPLOYE;
            EXECUTE IMMEDIATE 'INSERT INTO EMPLOYE (ID_EMPLOYE, NOM, PRENOM, EMAIL, ROLE, LOGIN, MDP, ACCES)
                               VALUES (:1, :2, :3, :4, :5, :6, :7, :8)'
                USING v_id, p_nom, p_prenom, p_login || '@hacktime.tn', p_role, p_login, p_login, p_acces;
            DBMS_OUTPUT.PUT_LINE('Compte ' || p_login || ' cree (' || p_acces || ').');
        END IF;
    END;
BEGIN
    -- 1. colonne ACCES (une seule fois)
    SELECT COUNT(*) INTO n FROM user_tab_columns WHERE table_name = 'EMPLOYE' AND column_name = 'ACCES';
    IF n = 0 THEN
        EXECUTE IMMEDIATE 'ALTER TABLE EMPLOYE ADD (ACCES VARCHAR2(20))';
        EXECUTE IMMEDIATE 'ALTER TABLE EMPLOYE ADD CONSTRAINT CK_EMPLOYE_ACCES CHECK (ACCES IN '
            || '(''TOUT'', ''COMPETITIONS'', ''EQUIPES'', ''PARTICIPANTS'', ''ATELIERS'', ''EMPLOYES''))';
        DBMS_OUTPUT.PUT_LINE('Colonne EMPLOYE.ACCES ajoutee.');
    END IF;

    -- 2. comptes (mot de passe = nom d'utilisateur)
    compte('hacktime123', 'Admin',   'HackTime', 'ADMINISTRATEUR', 'TOUT');
    compte('yosr123',     'Yosr',    'Yosr',     'ORGANISATEUR',   'COMPETITIONS');
    compte('mariem123',   'Mariem',  'Mariem',   'ORGANISATEUR',   'EQUIPES');
    compte('yasmine123',  'Yasmine', 'Yasmine',  'ORGANISATEUR',   'PARTICIPANTS');
    compte('adam123',     'Adam',    'Adam',     'ORGANISATEUR',   'ATELIERS');
    compte('mohamed123',  'Mohamed', 'Mohamed',  'ORGANISATEUR',   'EMPLOYES');
END;
/

COMMIT;

-- Verification :
-- SELECT NOM, ROLE, LOGIN, MDP, ACCES FROM EMPLOYE ORDER BY ID_EMPLOYE;
