/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file QEthereum.h
 * @authors:
 *   Gav Wood <i@gavwood.com>
 *   Marek Kotewicz <marek@ethdev.com>
 * @date 2014
 */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <jsonrpc/rpc.h>

class QWebThree: public QObject
{
	Q_OBJECT
	
public:
	QWebThree(QObject* _p);
	virtual ~QWebThree();

	Q_INVOKABLE void postData(QString _json);
	
signals:
	void processData(QString _json);
	void send(QString _json);
};

class QWebThreeConnector: public QObject, public jsonrpc::AbstractServerConnector
{
	Q_OBJECT
	
public:
	QWebThreeConnector(QWebThree* _q);
	virtual ~QWebThreeConnector();
	
	virtual bool StartListening();
	virtual bool StopListening();
	
	bool virtual SendResponse(std::string const& _response, void* _addInfo = NULL);
	
public slots:
	void onMessage(QString const& _json);
	
private:
	QWebThree* m_qweb;
};

#define QETH_INSTALL_JS_NAMESPACE(_frame, _env, qweb) [_frame, _env, qweb]() \
{ \
	_frame->disconnect(); \
	_frame->addToJavaScriptWindowObject("_web3", qweb, QWebFrame::ScriptOwnership); \
	_frame->evaluateJavaScript("navigator.qt = _web3;"); \
	_frame->evaluateJavaScript("(function () {" \
							"navigator.qt.handlers = [];" \
							"Object.defineProperty(navigator.qt, 'onmessage', {" \
							"	set: function(handler) {" \
							"		navigator.qt.handlers.push(handler);" \
							"	}" \
							"})" \
							"})()"); \
	_frame->evaluateJavaScript("navigator.qt.send.connect(function (res) {" \
							"navigator.qt.handlers.forEach(function (handler) {" \
							"	handler(res);" \
							"})" \
							"})"); \
}


