#include "livemessagetoken.h"

#include <QDebug>

LiveMessageToken::LiveMessageToken() {
	valueType = Invalid; 
}

LiveMessageToken::LiveMessageToken(const QString &value) {
	valueType = String;
	stringVal = value;
}

QByteArray LiveMessageToken::toByteArray() const {
	if (valueType == Int) {
		return QString("i%1e").arg(intVal).toLocal8Bit();
	} else if (valueType == List) {
		QByteArray retVal("l");
		foreach(LiveMessageToken token, listVal) {
			retVal.append(token.toByteArray());
		}
		retVal.append("s");
		return retVal;
	} else if (valueType == Dictionary) {
		QByteArray retVal("h");
		QHashIterator<QString, LiveMessageToken> it(dictVal);
		while(it.hasNext()) {
			it.next();
			LiveMessageToken key(it.key());
			retVal.append(key.toByteArray());
			retVal.append(it.value().toByteArray());
		}
		retVal.append("s");
		return retVal;
	}
	return QString("%1:%2").arg(stringVal.length(), 0, 16).arg(stringVal).toLocal8Bit();
}

LiveMessageToken LiveMessageToken::parseToken(const QByteArray &string, int* start) {
	LiveMessageToken d;
	if ((*start) >= string.length()) {
		return d;
	}
	if (string[(*start)] == 'i') { //Int
		qDebug() << "INT!";
	} else { //String
		int index = string.indexOf(':', (*start));
		if (index < 0) {
			qDebug() << "Error in" << string;
			return d;
		}
		bool ok;
		int length = string.mid((*start), index - (*start)).toInt(&ok, 16);
		if (!ok) {
			return d;
		}
		d.stringVal = string.mid(index+1, length);
		d.valueType = LiveMessageToken::String;
		(*start) = index + length + 1;
	}
	return d;
}

LiveMessageTokenScriptWrapper::LiveMessageTokenScriptWrapper()
	:QObject()
{
}

LiveMessageTokenScriptWrapper::~LiveMessageTokenScriptWrapper() {
}

LiveMessageToken LiveMessageTokenScriptWrapper::token() const {
	return p_token;
}

void LiveMessageTokenScriptWrapper::add(LiveMessageTokenScriptWrapper *t) {
	p_token.valueType = LiveMessageToken::List;
	p_token.listVal << t->p_token;
}

void LiveMessageTokenScriptWrapper::set(const QString &key, int value) {
	this->set(key, QString::number(value));
}

void LiveMessageTokenScriptWrapper::set(const QString &key, const QString &value) {
	p_token.valueType = LiveMessageToken::Dictionary;
	p_token.dictVal[key] = LiveMessageToken(value);
}
