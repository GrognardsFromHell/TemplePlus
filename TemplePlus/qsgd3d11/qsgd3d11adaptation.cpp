/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsgd3d11adaptation_p.h"
#include "qsgd3d11renderloop_p.h"
#include "qsgd3d11context_p.h"

QT_BEGIN_NAMESPACE

QSGD3D11Adaptation::QSGD3D11Adaptation(QObject *parent)
    : QSGContextPlugin(parent)
{
}

QStringList QSGD3D11Adaptation::keys() const
{
    return QStringList() << QLatin1String("d3d11");
}

QSGContext *QSGD3D11Adaptation::create(const QString &) const
{
    if (!contextInstance)
        contextInstance = new QSGD3D11Context;

    return contextInstance;
}

QSGContextFactoryInterface::Flags QSGD3D11Adaptation::flags(const QString &) const
{
    return 0;
}

QSGRenderLoop *QSGD3D11Adaptation::createWindowManager()
{
	qFatal("createWindowManager() should never be called, since we set the render loop via QSGRenderLoop::setInstance");
	return nullptr;
}

QSGD3D11Context *QSGD3D11Adaptation::contextInstance = nullptr;

QT_END_NAMESPACE
