/*
 *
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */
package com.fingerprints.imagecollection.interfaces;

import com.fingerprints.imagecollection.scenario.ImageCollectionConfig;

public interface IConfigurationManager {
    public ImageCollectionConfig getConfiguration() throws Exception;

    public boolean hasExternalConfiguration();
}
