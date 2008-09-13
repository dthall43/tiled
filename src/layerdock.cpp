/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "layerdock.h"

#include "layer.h"
#include "layertablemodel.h"
#include "map.h"
#include "mapdocument.h"
#include "propertiesdialog.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QTreeView>
#include <QUndoStack>

using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

/**
 * This view makes sure the size hint makes sense and implements the context
 * menu.
 */
class LayerView : public QTreeView
{
    public:
        LayerView(QUndoStack *undoStack, QWidget *parent = 0);

        QSize sizeHint() const
        {
            return QSize(130, 100);
        }

        void contextMenuEvent(QContextMenuEvent *event);

    private:
        QUndoStack *mUndoStack;
};

} // namespace Internal
} // namespace Tiled


LayerDock::LayerDock(QUndoStack *undoStack, QWidget *parent):
    QDockWidget(tr("Layers"), parent),
    mMapDocument(0)
{
    setObjectName(QLatin1String("layerDock"));

    mLayerView = new LayerView(undoStack, this);
    setWidget(mLayerView);
}

void LayerDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument) {
        mMapDocument->disconnect(this);
        mLayerView->selectionModel()->disconnect(this);
    }

    mMapDocument = mapDocument;

    if (mMapDocument) {
        mLayerView->setModel(mapDocument->layerModel());

        connect(mMapDocument, SIGNAL(currentLayerChanged(int)),
                this, SLOT(currentLayerChanged(int)));

        QItemSelectionModel *s = mLayerView->selectionModel();
        connect(s, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
                this, SLOT(currentRowChanged(QModelIndex)));

        currentLayerChanged(mMapDocument->currentLayer());
    } else {
        mLayerView->setModel(0);
    }
}

void LayerDock::currentRowChanged(const QModelIndex &index)
{
    const int layer = mMapDocument->layerModel()->toLayerIndex(index);
    mMapDocument->setCurrentLayer(layer);
}

void LayerDock::currentLayerChanged(int index)
{
    if (index > -1) {
        const LayerTableModel *layerModel = mMapDocument->layerModel();
        const int layerCount = layerModel->rowCount();
        const int row = layerCount - index - 1;
        mLayerView->setCurrentIndex(layerModel->index(row, 0));
    } else {
        mLayerView->setCurrentIndex(QModelIndex());
    }
}


LayerView::LayerView(QUndoStack *undoStack, QWidget *parent):
    QTreeView(parent),
    mUndoStack(undoStack)
{
    setRootIsDecorated(false);
    setHeaderHidden(true);
    setItemsExpandable(false);
    setUniformRowHeights(true);
}

void LayerView::contextMenuEvent(QContextMenuEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    const LayerTableModel *m = static_cast<LayerTableModel*>(model());
    if (!m)
        return;
    const int layerIndex = m->toLayerIndex(index);
    if (layerIndex < 0)
        return;

    QMenu menu;
    QAction *layerProperties = menu.addAction(tr("Properties..."));

    if (menu.exec(event->globalPos()) == layerProperties) {
        Layer *layer = m->map()->layers().at(layerIndex);

        PropertiesDialog propertiesDialog(mUndoStack, this);
        propertiesDialog.setProperties(layer->properties());
        propertiesDialog.exec();
    }
}
