#include "ontologydb.h"
#include <QSqlQuery>
#include <QVariant>
using namespace std;

OntologyDB::OntologyDB(QObject *parent) :
    QObject(parent)
{
}

bool OntologyDB::openDB(){
    // Find QSLite driver
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString path("/run/shm/ontology.db");
    db.setDatabaseName(path);
    // Open databasee
    return db.open();
}

void OntologyDB::closeDB()
{
    if(db.isOpen())db.close();
}

int OntologyDB::getOntologyID(QString filepath)
{
    if(!db.isOpen())return -1;
    int oid=0;
    QSqlQuery sq;
    sq.prepare("Select ONTOLOGYID from ontologies where description = :path");
    sq.bindValue(":path",filepath);
    sq.exec();
    while(sq.next()){
        oid = sq.value(0).toInt();
    }
    cout<<"DB get ontologyID ="<<oid<<endl;
    return oid;
}

QString OntologyDB::getOntologyNamespace(int ontoID)
{
    if(!db.isOpen())return NULL;
    QString ns="";
    QSqlQuery sq;
    sq.prepare("Select NAMESPACE from ontologies where ontologyID = :oid");
    sq.bindValue(":oid",ontoID);
    sq.exec();
    while(sq.next()){
        ns = sq.value(0).toString();
    }
    cout<<"DB get NS: " <<ns.toStdString()<<endl;
    return ns;
}

QList<OwlIndividual *> OntologyDB::getAllIndividuals(int ontoID)
{
    QList<OwlIndividual *> idvs;
    if(!db.isOpen())return idvs;

    QSqlQuery sq;
    sq.prepare("Select URI,NAME,entityid from individuals where ontologyID = :oid");
    sq.bindValue(":oid",ontoID);
    sq.exec();
    while(sq.next()){
        OwlIndividual * tmp = new OwlIndividual();
        tmp->URI = sq.value(0).toString();
        tmp->shortname = sq.value(1).toString();
        tmp->db_entityID = sq.value(2).toInt();

        //create shape
        tmp->shape = new OntologyIndividualShape();
        tmp->shape->setIdString(tmp->shortname);
        tmp->shape->setLabel(tmp->shortname);
        tmp->shape->setToolTip(tmp->URI);
        tmp->shape->setSize(QSizeF(150,20));
        tmp->shape->setFillColour(OwlIndividual::INDIVIDUAL_SHAPE_COLOR);

        idvs.append(tmp);
    }
    cout<<"DB individuals: "<<idvs.size()<<endl;
    return idvs;
}

QList<OwlClass *> OntologyDB::getAllNamedClasses(int ontoID)
{
    QList<OwlClass *> cls;
    if(!db.isOpen())return cls;

    QSqlQuery sq;
    sq.prepare("Select URI,SHORTNAME,EntityID from classes where ontologyID = :oid");
    sq.bindValue(":oid",ontoID);
    sq.exec();
    int i=0;
    while(sq.next()){
        OwlClass * tmpclass = new OwlClass();
        tmpclass->URI = sq.value(0).toString();
        tmpclass->shortname = sq.value(1).toString();
        tmpclass->db_entityID = sq.value(2).toInt();

        //create shape
        tmpclass->shape = new OntologyClassShape();
        tmpclass->shape->setIdString(tmpclass->shortname);
        tmpclass->shape->setToolTip(tmpclass->URI);
        tmpclass->shape->setPosAndSize(QPointF(0,i*25),QSizeF(150,20));
        i++;
        tmpclass->shape->setMyLabel(tmpclass->shortname);
        tmpclass->shape->setLabelByLevels(1,tmpclass->shortname); //set level 1 label
        tmpclass->shape->setLabelByLevels(2,"[IND]:");
        tmpclass->shape->setLabelByLevels(3,"[SUB]:");
        tmpclass->shape->setLabelByLevels(4,"[SUP]:");
        tmpclass->shape->setLabelByLevels(5,"[DIS]:");
        tmpclass->shape->setLabelByLevels(6,"[EQU]:");
        tmpclass->shape->setFillColour(OwlClass::CLASS_SHAPE_COLOR);

        cls.append(tmpclass);
    }
    return cls;
}

QList<int> OntologyDB::getIndividualURIsByClass(int class_entityid)
{
//    cout<<"get IND..."<<endl;
    QList<int> idvs;
    if(!db.isOpen())return idvs;
    QSqlQuery sq;
    sq.prepare("select individualID from individualrelations where classID = :clsid");
    sq.bindValue(":clsid",class_entityid);
    sq.exec();
    while(sq.next())
    {
        idvs += sq.value(0).toInt();
    }
    return idvs;
}

QList<int> OntologyDB::getSubClasses(int class_entityid)
{
//    cout<<"get subs..."<<endl;
    QList<int> subs;
    if(!db.isOpen())return subs;

    QSqlQuery sq;

    sq.prepare("select childid from subSumption where parentID= :clsid");
    sq.bindValue(":clsid",class_entityid);
    sq.exec();
    while(sq.next())
    {
        subs += sq.value(0).toInt();
    }
    return subs;

}

QList<int> OntologyDB::getSuperClasses(int class_entityid){

//    cout<<"get sups..."<<endl;
    QList<int> sups;
    if(!db.isOpen())return sups;

    QSqlQuery sq;

    sq.prepare("select parentid from subSumption where childID= :clsid");
    sq.bindValue(":clsid",class_entityid);
    sq.exec();
    while(sq.next())
    {
        sups += sq.value(0).toInt();
    }
    return sups;
}

QList<int> OntologyDB::getDisjointClasses(int class_entityid){

//    cout<<"get diss..."<<endl;
    QList<int> disjoints;
    if(!db.isOpen())return disjoints;

    QSqlQuery sq;
    sq.prepare("select disjointClassID from disjointClasses where classID = :clsid;");
    sq.bindValue(":clsid",class_entityid);
    sq.exec();

    while(sq.next())
    {
        disjoints += sq.value(0).toInt();
    }

    return disjoints;
}

QList<int> OntologyDB::getEquivalentClasses(int class_entityid){

//    cout<<"get equs..."<<endl;
    QList<int> equs;
    if(!db.isOpen())return equs;

    QSqlQuery sq;
    sq.prepare("select EquivalentClassID from EquivalentClasses where classID = :clsid;");
    sq.bindValue(":clsid",class_entityid);
    sq.exec();

    while(sq.next())
    {
        equs += sq.value(0).toInt();
    }

    return equs;
}


QList<QString> OntologyDB::getAnonymousSubClasses(int class_entityid)
{

//    cout<<"get asubs..."<<endl;
    QList<QString> asubs;
    if(!db.isOpen())return asubs;

    QSqlQuery sq;
//    sq.prepare("select anonymousclasses.anonymousexpression from (select * from subSumption where parentID = :clsid) as t1 join anonymousclasses on t1.childid = anonymousclasses.entityID;");
    sq.prepare("select anonymousclasses.anonymousexpression from subSumption join anonymousclasses on childid = entityID where parentID = :clsid;");
    sq.bindValue(":clsid",class_entityid);
    sq.exec();

    while(sq.next())
    {
        asubs += sq.value(0).toString();
    }

    return asubs;
}

QList<QString> OntologyDB::getAnonymousSuperClasses(int class_entityid)
{

//    cout<<"get asups..."<<endl;
    QList<QString> asups;
    if(!db.isOpen())return asups;

    QSqlQuery sq;
//    sq.prepare("select anonymousclasses.anonymousexpression from (select * from subSumption where childID = :clsid) as t1 join anonymousclasses on t1.parentID = anonymousclasses.entityID;");
    sq.prepare("select anonymousclasses.anonymousexpression from subSumption join anonymousclasses on parentID = entityID where childID = :clsid;");
    sq.bindValue(":clsid",class_entityid);
    sq.exec();

    while(sq.next())
    {
        asups += sq.value(0).toString();
    }

    return asups;
}

QList<QString> OntologyDB::getAnonymousDisjointClasses(int class_entityid)
{

//    cout<<"get adiss..."<<endl;
    QList<QString> adisjoint;
    if(!db.isOpen())return adisjoint;

    QSqlQuery sq;
//    sq.prepare("select anonymousclasses.anonymousexpression from (select * from disjointClasses where classID = :clsid) as t1 join anonymousclasses on t1.disjointClassID = anonymousclasses.entityID;");
    sq.prepare("select anonymousclasses.anonymousexpression from disjointClasses join anonymousclasses on disjointClassID = entityID where classID = :clsid;");
    sq.bindValue(":clsid",class_entityid);
    sq.exec();

    while(sq.next())
    {
        adisjoint += sq.value(0).toString();
    }

    return adisjoint;
}

QList<QString> OntologyDB::getAnonymousEquivalentClasses(int class_entityid)
{

//    cout<<"get aequs..."<<endl;
    QList<QString> aequs;
    if(!db.isOpen())return aequs;

    QSqlQuery sq;
//    sq.prepare("select anonymousclasses.anonymousexpression from (select * from EquivalentClasses where classID = :clsid) as t1 join anonymousclasses on t1.EquivalentClassID = anonymousclasses.entityID;");
    sq.prepare("select anonymousclasses.anonymousexpression from EquivalentClasses join anonymousclasses on EquivalentClassID = entityID where classid = :clsid;");
    sq.bindValue(":clsid",class_entityid);
    sq.exec();

    while(sq.next())
    {
        aequs += sq.value(0).toString();
    }

    return aequs;
}

QList<OwlProperty *> OntologyDB::getAllNamedProperties(int ontoID){

}

QList<int> OntologyDB::getSubProperties(int Property_entityid){

}

QList<int> OntologyDB::getSuperProperties(int Property_entityid){

}

QList<int> OntologyDB::getDisjointProperties(int Property_entityid){

}

QList<int> OntologyDB::getEquivalentProperties(int Property_entityid){

}

QList<QString> OntologyDB::getAnonymousSubProperties(int Property_entityid){

}

QList<QString> OntologyDB::getAnonymousSuperProperties(int Property_entityid){

}

QList<QString> OntologyDB::getAnonymousDisjointProperties(int Property_entityid){

}

QList<QString> OntologyDB::getAnonymousEquivalentProperties(int Property_entityid){

}

