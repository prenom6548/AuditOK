/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "context.h"
#include <libkywc.h>

class Thumbnail::Private
{
  public:
    Private(Thumbnail *thumbnail);
    ~Private();
    void setup(kywc_context *ctx, Thumbnail::Type type, QString uuid, QString output_uuid);

    int32_t fd;
    uint32_t format;
    QSize size;
    uint32_t offset;
    uint32_t stride;
    uint64_t modifier;
    Thumbnail::BufferFlags flags;

    Type type;
    QString source_uuid;
    QString output_uuid;

    kywc_thumbnail *k_thumbnail;

  private:
    Thumbnail *t;
    static bool bufferHandle(kywc_thumbnail *thumbnail, const struct kywc_thumbnail_buffer *buffer,
                             void *data);
    static void destroyHandle(kywc_thumbnail *thumbnail, void *data);
    static struct kywc_thumbnail_interface thumbnail_impl;
};

Thumbnail::Private::Private(Thumbnail *thumbnail) : t(thumbnail) {}

Thumbnail::Private::~Private() {}

bool Thumbnail::Private::bufferHandle(kywc_thumbnail *thumbnail,
                                      const struct kywc_thumbnail_buffer *buffer, void *data)
{
    Thumbnail *thum = (Thumbnail *)data;

    switch (thumbnail->type) {
    case KYWC_THUMBNAIL_TYPE_OUTPUT:
        thum->pri->type = Thumbnail::Type::Output;
        break;
    case KYWC_THUMBNAIL_TYPE_TOPLEVEL:
        thum->pri->type = Thumbnail::Type::Toplevel;
        break;
    case KYWC_THUMBNAIL_TYPE_WORKSPACE:
        thum->pri->type = Thumbnail::Type::Workspace;
        break;
    }

    thum->pri->source_uuid = QString(thumbnail->source_uuid);
    thum->pri->output_uuid = QString(thumbnail->output_uuid);

    if (buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_DMABUF) {
        thum->pri->flags |= Thumbnail::BufferFlag::Dmabuf;
    }

    if (buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_REUSED) {
        thum->pri->flags |= Thumbnail::BufferFlag::Reused;
    }

    thum->pri->size = QSize(buffer->width, buffer->height);
    thum->pri->offset = buffer->offset;
    thum->pri->stride = buffer->stride;
    thum->pri->fd = buffer->fd;

    thum->pri->format = buffer->format;
    thum->pri->modifier = buffer->modifier;

    emit thum->bufferUpdate();

    if (buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_DMABUF) {
        return true;
    } else
        return false;
}

void Thumbnail::Private::destroyHandle(kywc_thumbnail *thumbnail, void *data)
{
    Thumbnail *thum = (Thumbnail *)data;
    emit thum->deleted();
}

struct kywc_thumbnail_interface Thumbnail::Private::thumbnail_impl {
    bufferHandle, destroyHandle,
};

void Thumbnail::Private::setup(kywc_context *ctx, Thumbnail::Type type, QString uuid,
                               QString output_uuid)
{
    enum kywc_thumbnail_type flag = KYWC_THUMBNAIL_TYPE_TOPLEVEL;
    switch (type) {
    case Thumbnail::Type::Output:
        flag = KYWC_THUMBNAIL_TYPE_OUTPUT;
        break;
    case Thumbnail::Type::Toplevel:
        flag = KYWC_THUMBNAIL_TYPE_TOPLEVEL;
        break;
    case Thumbnail::Type::Workspace:
        flag = KYWC_THUMBNAIL_TYPE_WORKSPACE;
        break;
    }
    QByteArray qByteArray_uuid = uuid.toUtf8();
    char *str = qByteArray_uuid.data();

    QByteArray qByteArray = output_uuid.toUtf8();
    char *output = qByteArray.data();
    kywc_thumbnail *thumbnail =
        kywc_thumbnail_create(ctx, flag, str, output, &thumbnail_impl, this->t);
    k_thumbnail = thumbnail;
}

Thumbnail::Thumbnail(QObject *parent) : pri(new Private(this)) {}

Thumbnail::~Thumbnail() {}

void Thumbnail::setup(kywc_context *ctx, Thumbnail::Type type, QString uuid, QString output_uuid)
{
    pri->setup(ctx, type, uuid, output_uuid);
}

int32_t Thumbnail::fd() const
{
    return pri->fd;
}

uint32_t Thumbnail::format() const
{
    return pri->format;
}

QSize Thumbnail::size() const
{
    return pri->size;
}

uint32_t Thumbnail::offset() const
{
    return pri->offset;
}

uint32_t Thumbnail::stride() const
{
    return pri->stride;
}

uint64_t Thumbnail::modifier() const
{
    return pri->modifier;
}

Thumbnail::BufferFlags Thumbnail::flags() const
{
    return pri->flags;
}

void Thumbnail::destory()
{
    if(pri->k_thumbnail) {
        kywc_thumbnail_destroy(pri->k_thumbnail);
        pri->k_thumbnail = nullptr;
    }
}
