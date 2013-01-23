#include "robomongo/core/mongodb/MongoClient.h"
#include "robomongo/core/domain/MongoNamespace.h"
#include <mongo/client/dbclient.h>

using namespace Robomongo;
using namespace std;

MongoClient::MongoClient(mongo::ScopedDbConnection *scopedConnection) :
    _scopedConnection(scopedConnection),
    _dbclient(scopedConnection->get()){ }

QStringList MongoClient::getCollectionNames(const QString &dbname)
{
    list<string> dbs = _dbclient->getCollectionNames(dbname.toStdString());

    QStringList stringList;
    for (list<string>::iterator i = dbs.begin(); i != dbs.end(); i++) {
        stringList.append(QString::fromStdString(*i));
    }

    stringList.sort();
    return stringList;
}

QStringList MongoClient::getDatabaseNames()
{
    list<string> dbs = _dbclient->getDatabaseNames();

    QStringList dbNames;
    for (list<string>::iterator i = dbs.begin(); i != dbs.end(); ++i) {
        dbNames.append(QString::fromStdString(*i));
    }

    dbNames.sort();
    return dbNames;
}

QList<MongoDocumentPtr> MongoClient::query(const MongoQueryInfo &info)
{
    MongoNamespace ns(info.databaseName, info.collectionName);

    QList<MongoDocumentPtr> docs;
    auto_ptr<mongo::DBClientCursor> cursor = _dbclient->query(
        ns.toString().toStdString(), info.query, info.limit, info.skip,
        info.fields.nFields() ? &info.fields : 0, info.options, info.batchSize);

    while (cursor->more()) {
        mongo::BSONObj bsonObj = cursor->next();
        MongoDocumentPtr doc(new MongoDocument(bsonObj.getOwned()));
        docs.append(doc);
    }

    return docs;
}

MongoCollectionInfo MongoClient::runCollStatsCommand(const QString &ns)
{
    MongoNamespace mongons(ns);

    mongo::BSONObjBuilder command; // { collStats: "db.collection", scale : 1 }
    command.append("collStats", mongons.collectionName().toStdString());
    command.append("scale", 1);

    mongo::BSONObj result;
    _dbclient->runCommand(mongons.databaseName().toStdString(), command.obj(), result);

    MongoCollectionInfo info(result);
    return info;
}

QList<MongoCollectionInfo> MongoClient::runCollStatsCommand(const QStringList &namespaces)
{
    QList<MongoCollectionInfo> infos;
    foreach (QString ns, namespaces) {
        MongoCollectionInfo info = runCollStatsCommand(ns);
        infos.append(info);
    }
    return infos;
}

void MongoClient::done()
{
    _scopedConnection->done();
}