/*
 * Fedora Media Writer
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "release.h"
#include "releasemanager.h"
#include "image_type.h"
#include "architecture.h"
#include "variant.h"

#include <QDebug>

Release::Release(ReleaseManager *parent, const QString &name, const QString &display_name, const QString &summary, const QString &description, const QString &icon, const QStringList &screenshots)
: QObject(parent)
, m_name(name)
, m_displayName(display_name)
, m_summary(summary)
, m_description(description)
, m_icon(icon)
, m_screenshots(screenshots)
, m_isCustom(false)
{
    // TODO: connect to release's signal in parent, not the other way around, won't need to have parent be releasemanager then
    connect(
        this, &Release::selectedVariantChanged,
        parent, &ReleaseManager::variantChangedFilter);
}

Release *Release::custom(ReleaseManager *parent) {
    auto customRelease = new Release(parent, QString(), tr("Custom image"), QT_TRANSLATE_NOOP("Release", "Pick a file from your drive(s)"), { QT_TRANSLATE_NOOP("Release", "<p>Here you can choose a OS image from your hard drive to be written to your flash disk</p><p>Currently it is only supported to write raw disk images (.iso or .bin)</p>") }, "qrc:/logo/custom", {});
    customRelease->m_isCustom = true;
    customRelease->setLocalFile(QString());

    return customRelease;
}

void Release::updateUrl(const QString &url, Architecture *architecture, ImageType *imageType, const QString &board, const bool live) {
    // If variant already exists, update it
    Variant *variant_in_list =
    [=]() -> Variant * {
        for (auto variant : m_variants) {
            // TODO: equals?
            if (variant->arch() == architecture && variant->imageType() == imageType && variant->board() == board && variant->live() == live) {
                return variant;
            }
        }
        return nullptr;
    }();
    if (variant_in_list != nullptr) {
        qDebug() << "Variant already loaded, only updating url";

        variant_in_list->updateUrl(url);
        return;
    }

    // Otherwise make a new variant

    // NOTE: preserve the order from the Architecture::Id enum (to not have ARM first, etc.)
    const int insert_index =
    [this, architecture]() {
        int out = 0;
        for (auto variant : m_variants) {
            // NOTE: doing pointer comparison because architectures are a singleton pointers
            if (variant->arch() > architecture) {
                return out;
            }

            out++;
        }
        return out;
    }();
    auto new_variant = new Variant(url, architecture, imageType, board, live, this);

    m_variants.insert(insert_index, new_variant);
    emit variantsChanged();
    
    // Select first variant by default
    if (m_variants.count() == 1) {
        m_selectedVariant = 0;
        emit selectedVariantChanged();
    }
}

void Release::setLocalFile(const QString &path) {
    // Delete old custom variant (there's only one, but iterate anyway)
    for (auto variant : m_variants) {
        variant->deleteLater();
    }
    m_variants.clear();

    // Add new variant
    ImageType *image_type = ImageType::fromFilename(path);
    auto customVariant = new Variant(path, Architecture::fromId(Architecture::UNKNOWN), image_type, QString(), false, this);
    // NOTE: start out in ready because don't need to download
    customVariant->setStatus(Variant::READY);
    m_variants.append(customVariant);
    
    emit variantsChanged();
    emit selectedVariantChanged();
 }

QString Release::name() const {
    return m_name;
}

QString Release::displayName() const {
    return m_displayName;
}

QString Release::summary() const {
    return tr(m_summary.toUtf8());
}

QString Release::description() const {
    return m_description;
}

bool Release::isCustom() const {
    return m_isCustom;
}

QString Release::icon() const {
    return m_icon;
}

QStringList Release::screenshots() const {
    return m_screenshots;
}

Variant *Release::selectedVariant() const {
    if (m_selectedVariant >= 0 && m_selectedVariant < m_variants.count())
        return m_variants[m_selectedVariant];
    return nullptr;
}

int Release::selectedVariantIndex() const {
    return m_selectedVariant;
}

void Release::setSelectedVariantIndex(int o) {
    if (m_selectedVariant != o && m_selectedVariant >= 0 && m_selectedVariant < m_variants.count()) {
        m_selectedVariant = o;
        emit selectedVariantChanged();
    }
}

QQmlListProperty<Variant> Release::variants() {
    return QQmlListProperty<Variant>(this, m_variants);
}

QList<Variant *> Release::variantList() const {
    return m_variants;
}
